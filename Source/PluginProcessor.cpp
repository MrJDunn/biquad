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
{
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

    for (int i = 0; i < numchannels; ++i)
    {
        x1[i] = x2[i] = y1[i] = y2[i] = 0;
    }

    calculateCoefficients(100.f);
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

void BiquadAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* in = buffer.getReadPointer (channel);
        auto* out = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float x0 = in[sample];
            float y0 = (x0 * d0) + c0 * (a1 * x1[channel] + a2 * x2[channel] - b1 * y1[channel] - b2 * y2[channel]);
            out[sample] = y0;

            x2[channel] = x1[channel];
            x1[channel] = x0;
            y2[channel] = y1[channel];
            y1[channel] = y0;
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
}

void BiquadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void BiquadAudioProcessor::calculateCoefficients(float frequency)
{
    // 1st Order LPF
    float theta = juce::MathConstants<float>::twoPi * frequency / samplerate;
    float gamma = cos(theta) / (1.f + sin(theta));
    a0 = (1.f - gamma) / 2.f;
    a1 = a0;
    a2 = 0.f;
    b1 = -gamma;
    b2 = 0.f;
    c0 = 1.f;
    d0 = 0.f;
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BiquadAudioProcessor();
}
