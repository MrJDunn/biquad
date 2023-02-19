/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/

#define DEBUG_NOISE

class BiquadAudioProcessor  : public AudioProcessor
{
public:
    enum FilterType
    {
        FirstOrderLPF = 0,
        FirstOrderHPF,
        SecondOrderLPF,
        SecodOrderHPF,
        SecondOrderBPF,
        SecondOrderBSF,
        SecondOrderButterworthLPF,
        SecondOrderButterworthHPF,
        SecondOrderButterworthBPF,
        SecondOrderButterworthBSF,
        SecondOrderLinkwitzRileyLPF,
        SecondOrderLinkwitzRileyHPF,
        FirstOrderAPF,
        SecondOrderAPF,
        FirstOrderHighShelf,
        FirstOrderLowShelf,
        SecondOrderParametricNonConstQ,
        SecondOrderParametricConstQ,
        FirstOrderAllPole,
        SecondOrderAllPole,
        SecondOrderVAMMLPF,
        SecondOrderVAMMBPF,
        FirstOrderIILPF,
        SecondOrderIILPF,
        FILTERTYPE_MAX
    };

    enum Params
    {
        FREQUENCY = 0,
        Q,
        GAIN,
        FILTERTYPE,
        PARAMS_MAX
    };

    //==============================================================================
    BiquadAudioProcessor();
    ~BiquadAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    StringArray getFilterTypeStrings();
private:
    float samplerate;
    float a0, a1, a2, b0, b1, b2, c0, d0;
    float *x1 = nullptr, *x2 = nullptr, *y1 = nullptr, *y2 = nullptr;

    juce::AudioParameterFloat *frequency, *q, *gain;
    juce::AudioParameterChoice *filterType;

    float mFrequency, mQ, mGain;
    FilterType mFilterType;

    void calculateCoefficients(float frequency, float q, float gain, FilterType filtertype);
    float linearToDecibels(const float &linearValue);
    float decibelsToLinear(const float &decibelValue);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BiquadAudioProcessor)
};
