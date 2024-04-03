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
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", AudioChannelSet::stereo(), true)
#endif
	)
#endif
	, forwardFFT(fftOrder)
	, window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
	addParameter(frequency = new juce::AudioParameterFloat("frequency", "F", juce::NormalisableRange<float>(20.f, 20000.f), 10000.f));
	addParameter(q = new juce::AudioParameterFloat("q", "Q", juce::NormalisableRange<float>(1.0f, 20.f), 1.0f));
	addParameter(gain = new juce::AudioParameterFloat("gain", "G", juce::NormalisableRange<float>(-5.f, 5.f), 0.0f));
	addParameter(filterType = new juce::AudioParameterChoice("filterType", "T", getFilterTypeStrings(), 0));
    
	*frequency = 10000.f;
	*q = 1.f;
	*gain = 0.f;
	*filterType = 0;

	std::fill(std::begin(scopeData), std::end(scopeData), 0.f);

	for (int i = 0; i < 600; ++i)
	{
		responseKeyFrequencies[i] = i / 20000.0f;
	}
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
/*
void FMOD_MultibandEQ_CalculateResponse(FMOD_DSP_MULTIBAND_EQ_FILTER_TYPE filter, const float coefficients[MAX_COEFS], int sampleRate, int count, const float *frequency, float *magnitude, float *phase)
{
    FMOD_DEBUG_ASSERT(coefficients);
    FMOD_DEBUG_ASSERT(sampleRate >= 8000);
    FMOD_DEBUG_ASSERT(count > 0);
    FMOD_DEBUG_ASSERT(frequency);
    FMOD_DEBUG_ASSERT(magnitude);
    FMOD_DEBUG_ASSERT(phase);

    float b0 = coefficients[0];
    float b1 = coefficients[1];
    float b2 = coefficients[2];
    float a0 = 1.0f;
    float a1 = coefficients[3];
    float a2 = coefficients[4];

    float rolloffMultiply = 1.0f;
    if (filter == FMOD_DSP_MULTIBAND_EQ_FILTER_LOWPASS_24DB || filter == FMOD_DSP_MULTIBAND_EQ_FILTER_HIGHPASS_24DB)
    {
        rolloffMultiply = 2.0f;
    }
    else if (filter == FMOD_DSP_MULTIBAND_EQ_FILTER_LOWPASS_48DB || filter == FMOD_DSP_MULTIBAND_EQ_FILTER_HIGHPASS_48DB)
    {
        rolloffMultiply = 4.0f;
    }

    float unwrapRange = 360.0f * rolloffMultiply;
    if (filter == FMOD_DSP_MULTIBAND_EQ_FILTER_NOTCH)
    {
        unwrapRange = 0.0f;
    }

    float nyquist = sampleRate / 2.0f;
    for (int i = 0; i < count; i++)
    {
        float normFreq = frequency[i] / nyquist;
        normFreq = FMOD_CLAMP(normFreq, 0.0001f, 0.9999f); // Slightly higher range than algorithm for better visualization

        FMOD_COMPLEX z1 = FMOD_COMPLEX_POLAR(1.0f, -FMOD_PI * normFreq);
        FMOD_COMPLEX num = b0 + (b1 + b2 * z1) * z1;
        FMOD_COMPLEX den = a0 + (a1 + a2 * z1) * z1;
        FMOD_COMPLEX res = num / den;

        magnitude[i] = FMOD_LINEAR_TO_DECIBELS(FMOD_COMPLEX_ABS(res)) * rolloffMultiply;
        phase[i] = FMOD_RAD_TO_DEG(FMOD_COMPLEX_ARG(res)) * rolloffMultiply;
    }

    if (unwrapRange)
    {
        FMOD_MultibandEQ_UnwrapPhase(count, phase, unwrapRange);
    }
}
*/
//void BiquadAudioProcessor::calculateResponse(int sampleRate, int count, const float* frequencies, float* magnitude, float* phase)
//{
//	//float nyquist = sampleRate / 2.0f;
//	for (int i = 0; i < count; i++)
//	{
//		float omega = (MathConstants<float>::twoPi * frequencies[i]) / sampleRate;
//		//float normFreq = frequencies[i] / nyquist;
//		omega = omega < 0.0001f ? 0.0001f : omega > 0.9999f ? 0.9999f : omega;
//
//		std::complex<float> z1 = exp(std::complex<float>(1.0f, omega));
//		std::complex<float> z2 = exp(std::complex<float>(1.0f, 2.f * omega));
//		std::complex<float> num = b0 + (b1 * z1) + (b2 * z2);
//		std::complex<float> den = a0 + (a1 * z1) * (a2 * z1);
//		std::complex<float> res = num / den;
//
//		magnitude[i] = linearToDecibels(abs(res.real()));
//		phase[i] = (res.real() * 180 / MathConstants<float>::pi);
//	}
//	return;
//}

void BiquadAudioProcessor::calculateResponse(int sampleRate, int count, const float* frequencies, float* magnitude, float* phase)
{
	std::complex<float> z1(0.0f);
	std::complex<float> z2(0.0f);
	float nyquist = sampleRate / 2.0f;
	for (int i = 0; i < count; i++)
	{
		//float omega = (MathConstants<float>::twoPi * frequencies[i]) / sampleRate;
		float normFreq = frequencies[i] / nyquist;
		float omega = normFreq < 0.0001f ? 0.0001f : normFreq  > 0.9999f ? 0.9999f : normFreq ;

		std::complex<float> num = b0 + (b1 * z1) + (b2 * z2);
		std::complex<float> den = a0 + (a1 * z1) * (a2 * z1);
		std::complex<float> res = num / den;
		z2 = z1;// exp(std::complex<float>(1.0f, 2.f * omega));
		z1 = omega;//  exp(std::complex<float>(1.0f, omega));

		magnitude[i] = linearToDecibels(abs(res.imag()));
		phase[i] = (res.real() * 180 / MathConstants<float>::pi);
	}
	return;
}

std::complex<float> BiquadAudioProcessor::evalCoefs_biquad3(std::vector<float> coefs, float freq) {
	float len = coefs.size();
	std::complex<float> res;
	for (int idx = 0; idx < len; idx++)
	{
		res += coefs[idx] * exp(std::complex<float>(0, -idx * freq * 2 * MathConstants<float>::pi));
	}
	return res;
}

void BiquadAudioProcessor::calcDataOptions() {
	bool maxSet = false;
	float maxMag = 0.0f;
	float freq = 0.0f;;
	float mag[600];
	float phase[600];
	int maxPoint = 600;
	for (int idx = 0; idx <= maxPoint; idx++) {
		// step through from 0-0.5 normalized frequency
		freq = idx / maxPoint * 0.5;

		auto resZeros = evalCoefs_biquad3(std::vector<float>{a0, a1, a2}, freq);
		std::complex<float> res;
		if (true/* (pretty sure is always true) poles.length > 0*/) {
			auto resPoles = evalCoefs_biquad3(std::vector<float>{b0,b1,b2}, freq);
			auto res = (resZeros / resPoles);
		}
		else
			res = resZeros;

		// magnitude (dB)
		auto tempMag = abs(res);
		if (tempMag == 0)
			tempMag = -300;
		else
			tempMag = 20 * log10(tempMag);

		if ((maxSet && (tempMag > maxMag)) || !maxSet) {
			maxMag = tempMag;
			maxSet = true;
		}

		// magnitude
		mag[idx] = (freq * samplerate, tempMag);	// dB

		// phase
		phase[idx] = (freq * samplerate, atan2(res.imag(), res.real()));
	}

	int i = 0;
	int broken = 2;
	// phase has too much error for responses near zero, fix them up
	//for (auto idx = 1; idx <= (maxPoint - 1); idx++) {
	//	if (mag[idx][1] <= -300) {
	//		auto p1 = (phase[idx - 1][1] + phase[idx + 1][1]) * 0.5;
	//		phase[idx][1] = p1;
	//	}
	//}

	// dB scaling
		// calc nearest 10 dB level above
	//	dBMax = Math.round(maxMag / 10 + 0.4999) * 10;
	//auto dBMin = dBMax - dBRange;
	//auto dBTicks = [dBMin, dBMin + dBRange * .25, dBMin + dBRange * .5, dBMin + dBRange * .75, dBMax];

	//	data = [
	//	{ data:mag }];
	//	options = {
	//		yaxis: { max: dBMax, min : dBMin, ticks : dBTicks, tickDecimals : 1 },
	//		xaxis : { noTicks: 5 },
	//	};
}

void BiquadAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	if (mFilter.fq != frequency->get()
		|| mFilter.qf != q->get()
		|| mFilter.gn != gain->get()
		|| mFilter.type != (FilterType)filterType->getIndex())
	{
		mFilter = Filter::create(frequency->get(),q->get(),gain->get(), (FilterType)filterType->getIndex());
		calculateCoefficients(mFilter);
		calculateResponse(samplerate, 600, responseKeyFrequencies, responseMagnitude, responsePhase);
		//calcDataOptions();
	}

    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

#ifdef DEBUG_UNIT_STEP
	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* out = buffer.getWritePointer(channel);
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			out[sample] = 0.0f;
			if (sample % 2 == 0)
				out[sample] = 1.0f;
		}
	}

#endif
#ifdef DEBUG_NOISE
	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* out = buffer.getWritePointer(channel);
		for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
		{
			out[sample] = (((rand() % 100) - 50) / 50.f) / 10.0f;
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
            float y0 = (a0 * x0) + c0 * (a1 * x1[channel] + a2 * x2[channel] - b1 * y1[channel] - b2 * y2[channel]);

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
	float fq = *frequency;
	float qf = *q;
	float gn = *gain;
    int ft = *filterType;

    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
	destData.append((void *)&fq, sizeof(float));
	destData.append((void *)&qf, sizeof(float));
	destData.append((void *)&gn, sizeof(float));
	destData.append((void *)&ft, sizeof(int));
}

void BiquadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	float fq;
	float qf;
	float gn;
    FilterType ft;

    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

	if (data)
	{
		MemoryBlock srcData(sizeInBytes, true);
		srcData.copyFrom(data, 0, sizeInBytes);
		srcData.copyTo((void*)&fq, 0, sizeof(float));
		srcData.copyTo((void*)&qf, 1 * sizeof(float), sizeof(float));
		srcData.copyTo((void*)&gn, 2 * sizeof(float), sizeof(float));
		srcData.copyTo((void*)&ft, 3 * sizeof(float), sizeof(int));

		*frequency = fq;
		*q = qf;
		*gain = gn;
		*filterType = ft;
	}
}

StringArray BiquadAudioProcessor::getFilterTypeStrings()
{
	return StringArray{
		"Low Pass",
		"High Pass",
		"Band Pass",
		"High Shelf",
		"Low Shelf",
		"Parametric"
	};
	/*return StringArray{
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
	};*/
}

void BiquadAudioProcessor::calculateCoefficients(const Filter& filter)
{
	calculateCoefficients(filter.fq, filter.qf, filter.gn, filter.type);
}

void BiquadAudioProcessor::calculateCoefficients(float frequency, float q, float gain, FilterType filtertype)
{ 
	for (int i = 0; i < sizeof(x1) / sizeof(float); ++i)
    {
        x1[i] = x2[i] = y1[i] = y2[i] = 0;
    }

	switch (filtertype)
	{
	case BiquadAudioProcessor::SecondOrderLPF:
	{
		float theta = juce::MathConstants<float>::twoPi * frequency / samplerate;
		jassert(q > 0.0f);
		float d = 1.f / q;
		float beta = 0.5f * ((1.f - (d / 2.f) * sin(theta)) / (1.f + (d / 2.f) * sin(theta)));
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
		float beta = 0.5f * ((1.f - (d / 2.f) * sin(theta)) / (1.f + (d / 2.f) * sin(theta)));
		float gamma = (0.5f + beta) * cos(theta);
		a0 = (0.5f + beta + gamma) / 2.f;
		a1 = -(0.5f + beta + gamma);
		a2 = a0;
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
	case BiquadAudioProcessor::FirstOrderHighShelf:
	{
		// Broken
		float theta = (juce::MathConstants<float>::twoPi * frequency) / samplerate;
		float mu = powf(10.f, gain / 20.f);
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
		float theta = juce::MathConstants<float>::twoPi * (frequency / samplerate);
		float mu = powf(10.f, gain / 20.f);
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
		float mu = powf(10.f, gain / 20.f);
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
