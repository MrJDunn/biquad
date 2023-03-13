/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BiquadAudioProcessor::BiquadAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
	, forwardFFT(fftOrder)
	, window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
	addParameter(frequency = new juce::AudioParameterFloat("frequency", "F", juce::NormalisableRange<float>(20.f, 20000.f), 10000.f));
	addParameter(q = new juce::AudioParameterFloat("q", "Q", juce::NormalisableRange<float>(0.01f, 10.f), 0.01f));
	addParameter(gain = new juce::AudioParameterFloat("gain", "G", juce::NormalisableRange<float>(-10.f, 10.f), 0.1f));
	addParameter(filterType = new juce::AudioParameterChoice("filterType", "T", getFilterTypeStrings(), 0));
    
	mFrequency = mQ = mGain = 0.f;
    FilterType mFilterType = FirstOrderLPF;
}

BiquadAudioProcessor::~BiquadAudioProcessor()
{
}

//==============================================================================
const String BiquadAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BiquadAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BiquadAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BiquadAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BiquadAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BiquadAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BiquadAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BiquadAudioProcessor::setCurrentProgram (int index)
{
}

const String BiquadAudioProcessor::getProgramName (int index)
{
    return {};
}

void BiquadAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void BiquadAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    int numchannels = getNumInputChannels();
    int nsize = numchannels * sizeof(float);
    samplerate = sampleRate;

	x1 = (float *)malloc(nsize);
	x2 = (float *)malloc(nsize);
	y1 = (float *)malloc(nsize);
	y2 = (float *)malloc(nsize);

	*frequency = mFrequency;
	*q = mQ;
	*gain = mGain;
	*filterType = mFilterType;

	calculateCoefficients(mFrequency, mQ, mGain, mFilterType);
}

void BiquadAudioProcessor::releaseResources()
{
    free(x1);
    free(x2);
    free(y1);
    free(y2);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BiquadAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BiquadAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	if (mFrequency != frequency->get()
		|| mQ != q->get()
		|| mGain != gain->get()
		|| mFilterType != (FilterType)filterType->getIndex())
	{
		mFrequency = frequency->get();
		mQ = q->get();
		mGain = gain->get();
		mFilterType = (FilterType)filterType->getIndex();

		calculateCoefficients(mFrequency, mQ, mGain, mFilterType);
	}

    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

#ifdef DEBUG_NOISE
	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* out = buffer.getWritePointer(channel);
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			out[sample] = ((rand() % 100) - 50) / 50.f;
		}
	}
#endif

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
		auto* in = buffer.getReadPointer(channel);
		auto* out = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
			float x0 = in[sample];
            float y0 = (x0 * d0) + c0 * (a1 * x1[channel] + a2 * x2[channel] - b1 * y1[channel] - b2 * y2[channel]);
            out[sample] = y0;

            x2[channel] = x1[channel];
            x1[channel] = x0;
            y2[channel] = y1[channel];
            y1[channel] = y0;

			//FFT
			if (channel == 0) // Sticking to one channel for now
			{
				if (fifoIndex == fftSize)
				{
					if (!nextFFTBlockReady)
					{
						juce::zeromem(fftData, sizeof(fftData));
						memcpy(fftData, fifo, sizeof(fifo));
						nextFFTBlockReady = true;
					}
					fifoIndex = 0;
				}
				fifo[fifoIndex++] = y0;
			}
			//FFT
        }
    }
}

//==============================================================================
bool BiquadAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BiquadAudioProcessor::createEditor()
{
    return new BiquadAudioProcessorEditor (*this);
}

//==============================================================================
void BiquadAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
	destData.append((void *)&mFrequency, sizeof(float));
	destData.append((void *)&mQ, sizeof(float));
	destData.append((void *)&mGain, sizeof(float));
	destData.append((void *)&mFilterType, sizeof(FilterType));
}

void BiquadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

	MemoryBlock srcData(sizeInBytes, true);
	srcData.copyFrom(data, 0, sizeInBytes);
	srcData.copyTo((void *)&mFrequency, 0, sizeof(float));
	srcData.copyTo((void *)&mQ, 1 * sizeof(float), sizeof(float));
	srcData.copyTo((void *)&mGain, 2 * sizeof(float), sizeof(float));
	srcData.copyTo((void *)&mFilterType, 3 * sizeof(float), sizeof(FilterType));
}

StringArray BiquadAudioProcessor::getFilterTypeStrings()
{
	return StringArray{
		"1 Ord LP",
		"1 Ord HP",
		"2 Ord LP",
		"2 Ord HP",
		"2 Ord BP",
		"2 Ord BS",
		"2 Ord Bwth LP",
		"2 Ord Bwth HP",
		"2 Ord Bwth BP",
		"2 Ord Bwth BS",
		"2 Ord LR LP",
		"2 Ord LR HP",
		"1 Ord AP",
		"2 Ord AP",
		"1 Ord H Shlf",
		"1 Ord L Shlf",
		"2 Ord Par Cnst",
		"2 Ord Par",
		"1 Ord All Pole",
		"2 Ord All Pole",
		"2 Ord VAMMLP",
		"2 Ord VAMMBP",
		"1 Ord IILP",
		"2 Ord IILP",
	};
}

void BiquadAudioProcessor::calculateCoefficients(float frequency, float q, float gain, FilterType filtertype)
{ 
	for (int i = 0; i < sizeof(x1) / sizeof(float); ++i)
    {
        x1[i] = x2[i] = y1[i] = y2[i] = 0;
    }

	switch (filtertype)
	{
	case BiquadAudioProcessor::FirstOrderLPF:
	{
		float theta = juce::MathConstants<float>::twoPi * frequency / samplerate;
		float gamma = cos(theta) / (1.f + sin(theta));
		a0 = (1.f - gamma) / 2.f;
		a1 = a0;
		a2 = 0.f;
		b1 = -gamma;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::FirstOrderHPF:
	{
		float theta = juce::MathConstants<float>::twoPi * frequency / samplerate;
		float gamma = cos(theta) / (1.f + sin(theta));
		a0 = (1.f + gamma) / 2.f;
		a1 = -a0;
		a2 = 0.f;
		b1 = -gamma;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderLPF:
	{
		float theta = juce::MathConstants<float>::twoPi * frequency / samplerate;
		jassert(q > 0.0f);
		float d = 1.f / q;
		float beta = 0.5f * (1.f - (d / 2.f) * sin(theta) / (1.f + (d / 2.f) * sin(theta)));
		float gamma = (0.5f + beta) * cos(theta);
		a0 = (0.5f + beta - gamma) / 2.f;
		a1 = 0.5f + beta - gamma;
		a2 = a0;
		b1 = -2.f * gamma;
		b2 = 2.f * beta;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecodOrderHPF:
	{
		float theta = juce::MathConstants<float>::twoPi * frequency / samplerate;
		jassert(q > 0.0f);
		float d = 1.f / q;
		float beta = 0.5f * (1.f - (d / 2.f) * sin(theta) / (1.f + (d / 2.f) * sin(theta)));
		float gamma = (0.5f + beta) * cos(theta);
		a0 = (0.5f + beta - gamma) / 2.f;
		a1 = -(0.5f + beta - gamma);
		a2 = a2;
		b1 = -2.f * gamma;
		b2 = 2.f * beta;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderBPF:
	{
		float kappa = tan((juce::MathConstants<float>::pi * frequency) / samplerate);
		float delta = (kappa * kappa) * q + kappa + q;
		a0 = kappa / delta;
		a1 = 0;
		a2 = -kappa / delta;
		b1 = (2.f * q * (kappa * kappa - 1.f)) / delta;
		b2 = ((kappa * kappa) * q - kappa + q) / delta;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderBSF:
	{
		float kappa = tan((juce::MathConstants<float>::pi * frequency) / samplerate);
		float delta = (kappa * kappa) * q + kappa + q;
		a0 = (q * (kappa * kappa + 1.f)) / delta;
		a1 = (2.f * q * (kappa * kappa - 1.f)) / delta;
		a2 = (q * (kappa * kappa + 1.f)) / delta;
		b1 = a1;
		b2 = ((kappa * kappa) * q - kappa + q) / delta;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderButterworthLPF:
	{
		float sigma = 1.f / tan((juce::MathConstants<float>::pi * frequency) / samplerate);
		a0 = 1.f / (1.f + juce::MathConstants<float>::sqrt2 * sigma + sigma * sigma);
		a1 = 2.f * a0;
		a2 = a0;
		b1 = 2.f * a0 * (1.f - sigma * sigma);
		b2 = a0 * (1.f - juce::MathConstants<float>::sqrt2 * sigma + sigma * sigma);
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderButterworthHPF:
	{
		float sigma = tan((juce::MathConstants<float>::pi * frequency) / samplerate);
		a0 = 1.f / (1.f + juce::MathConstants<float>::sqrt2 * sigma + sigma * sigma);
		a1 = -2.f * a0;
		a2 = a0;
		b1 = 2.f * a0 * (sigma * sigma - 1.f);
		b2 = a0 * (1.f - juce::MathConstants<float>::sqrt2 * sigma + sigma * sigma);
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderButterworthBPF:
	{
		jassert(q > 0.0f);
		float bandwidth = frequency / q;
		float sigma = 1.f / tan((juce::MathConstants<float>::pi * frequency * bandwidth) / samplerate);
		float delta = 2.f * cos((2.f * juce::MathConstants<float>::pi * frequency) / samplerate);
		a0 = 1.f / (1.f + sigma);
		a1 = 0.f;
		a2 = -a0;
		b1 = -a0 * (sigma * delta);
		b2 = a0 * (sigma - 1.f);
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderButterworthBSF:
	{
		jassert(q > 0.0f);
		float bandwidth = frequency / q;
		float sigma = tan((juce::MathConstants<float>::pi * frequency * bandwidth) / samplerate);
		float delta = 2.f * cos((2.f * juce::MathConstants<float>::pi * frequency) / samplerate);
		a0 = 1.f / (1.f + sigma);
		a1 = -a0 * delta;
		a2 = a0;
		b1 = a1;
		b2 = a0 * (1.f - sigma);
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderLinkwitzRileyLPF:
	{
		float omega = juce::MathConstants<float>::pi * frequency;
		float theta = omega / samplerate;
		float kappa = omega / tan(theta);
		float delta = kappa * kappa + omega * omega + 2.f * kappa * omega;
		a0 = (omega * omega) / delta;
		a1 = 2.f * a0;
		a2 = a0;
		b1 = ((-2.f * kappa * kappa) + (2.f * omega * omega)) / delta;
		b2 = ((-2.f * kappa * omega) + (kappa * kappa) + (omega * omega)) / delta;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderLinkwitzRileyHPF:
	{
		float omega = juce::MathConstants<float>::pi * frequency;
		float theta = omega / samplerate;
		float kappa = omega / tan(theta);
		float delta = kappa * kappa + omega * omega + 2.f * kappa * omega;
		a0 = (kappa * kappa) / delta;
		a1 = (-2.f * kappa * kappa) / delta;
		a2 = (kappa * kappa) / delta;
		b1 = ((-2.f * kappa * kappa) + (2.f * omega * omega)) / delta;
		b2 = ((-2.f * kappa * omega) + (kappa * kappa) + (omega * omega)) / delta;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::FirstOrderAPF:
	{
		float alpha = (tan((juce::MathConstants<float>::pi * frequency) / samplerate) -1.f ) / (tan((juce::MathConstants<float>::pi * frequency) / samplerate) + 1.f);
		a0 = alpha;
		a1 = 1.f;
		a2 = 0.f;
		b1 = alpha;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderAPF:
	{
		jassert(q > 0.0f);
		float bandwidth = frequency / q;
		float alpha = (tan((juce::MathConstants<float>::pi * bandwidth) / samplerate) -1.f ) / (tan((juce::MathConstants<float>::pi * bandwidth) / samplerate) + 1.f);
		float beta = -cos((juce::MathConstants<float>::twoPi * frequency) / samplerate);
		a0 = -alpha;
		a1 = beta * (1.f - alpha);
		a2 = 1.f;
		b1 = a1;
		b2 = -alpha;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::FirstOrderHighShelf:
	{
		// Broken
		float theta = (juce::MathConstants<float>::twoPi * frequency) / samplerate;
		float mu = gain;
		float beta = (1.f + mu) / 4.f;
		float delta = beta * tan(theta / 2.f);
		float gamma = (1.f - delta) / (1.f + delta);
		a0 = (1.f + gamma) / 2.f;
		a1 = -a0;
		a2 = 0.f;
		b1 = -gamma;
		b2 = 0.f;
		c0 = mu - 1.f;
		d0 = 1.f;

		break;
	}
	case BiquadAudioProcessor::FirstOrderLowShelf:
	{
		// Broken
		float theta = (juce::MathConstants<float>::twoPi * frequency) / samplerate;
		float mu = gain;
		float beta = 4.f / (1.f + mu);
		float delta = beta * tan(theta / 2.f);
		float gamma = (1.f - delta) / (1.f + delta);
		a0 = (1.f - gamma) / 2.f;
		a1 = a0;
		a2 = 0.f;
		b1 = -gamma;
		b2 = 0.f;
		c0 = mu - 1.f;
		d0 = 1.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderParametricNonConstQ:
	{
		// Broken
		float theta = (juce::MathConstants<float>::twoPi * frequency) / samplerate;
		float mu = gain;
		float zeta = 4.f / (1.f + mu);
		float beta = 0.5f * ((1.f - zeta * tan(theta / (2.f * q))) / (1.f + zeta * tan(theta / (2.f * q))));
		float gamma = (0.5f + beta) * cos(theta);
		a0 = 0.5f - beta;
		a1 = 0.f;
		a2 = -(0.5f - beta);
		b1 = -2.f * gamma;
		b2 = 2.f * beta;
		c0 = mu - 1.f;
		d0 = 1.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderParametricConstQ:
	{
		// Broken
		float kappa = tan((juce::MathConstants<float>::pi * frequency) / samplerate);
		float mu = gain;
		jassert(q > 0.f);
		float zeta = 1.f + 1.f / q * kappa + kappa * kappa;
		float beta = 2.f * (kappa * kappa - 1.f);
		float delta = 1.f - 1.f / q * kappa + kappa * kappa;
		if (gain > 0.f)
		{
			float alpha = 1.f + mu / q * kappa + kappa * kappa;
			float gamma = 1.f - mu / q * kappa + kappa * kappa;

			a0 = alpha / zeta;
			a1 = beta / zeta;
			a2 = gamma / zeta;
			b1 = a1;
			b2 = delta / zeta;
			c0 = 1.f;
			d0 = 0.f;
		}
		else
		{
			float alpha = 1.f + 1.f / (mu * q) * kappa + kappa * kappa;
			float gamma = 1.f - 1.f / (mu * q) * kappa + kappa * kappa;

			a0 = zeta / alpha;
			a1 = beta / alpha;
			a2 = delta / alpha;
			b1 = a1;
			b2 = gamma / alpha;
			c0 = 1.f;
			d0 = 0.f;
		}

		break;
	}
	case BiquadAudioProcessor::FirstOrderAllPole:
	{
		// Broken
		float theta = (juce::MathConstants<float>::twoPi * frequency) / samplerate;
		float gamma = 2.f - cos(theta);
		float delta = sqrt(gamma * gamma - 1.f - gamma);
		a0 = 1.f + delta;
		a1 = 0.f;
		a2 = 0.f;
		b1 = delta;
		b2 = 0.f;
		c0 = 0.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderAllPole:
	{
		a0 = 0.f;
		a1 = 0.f;
		a2 = 0.f;
		b1 = 0.f;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderVAMMLPF:
	{
		a0 = 0.f;
		a1 = 0.f;
		a2 = 0.f;
		b1 = 0.f;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderVAMMBPF:
	{
		a0 = 0.f;
		a1 = 0.f;
		a2 = 0.f;
		b1 = 0.f;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::FirstOrderIILPF:
	{
		a0 = 0.f;
		a1 = 0.f;
		a2 = 0.f;
		b1 = 0.f;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	case BiquadAudioProcessor::SecondOrderIILPF:
	{
		a0 = 0.f;
		a1 = 0.f;
		a2 = 0.f;
		b1 = 0.f;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	default:
	{
		a0 = 0.f;
		a1 = 0.f;
		a2 = 0.f;
		b1 = 0.f;
		b2 = 0.f;
		c0 = 1.f;
		d0 = 0.f;

		break;
	}
	}
}
float BiquadAudioProcessor::linearToDecibels(const float &linearValue)
{
	float decibels = 20.f * log10(linearValue);
	return decibels < 80.f ? 80.f : decibels > 0.f ? 0.f : decibels;
}
float BiquadAudioProcessor::decibelsToLinear(const float &decibelValue)
{
	return powf(10, decibelValue / 20.f);
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new BiquadAudioProcessor();
}
