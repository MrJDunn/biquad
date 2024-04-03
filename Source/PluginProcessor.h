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

//#define DEBUG_NOISE
//#define DEBUG_UNIT_STEP
#define DEBUG_UNIMPLEMENTED

class BiquadAudioProcessor  : public AudioProcessor
{
public:
    enum FilterType
    {
        SecondOrderLPF = 0,
        SecodOrderHPF,
        SecondOrderBPF,
        FirstOrderHighShelf,
        FirstOrderLowShelf,
        SecondOrderParametricNonConstQ,
        FILTERTYPE_MAX
    };
    struct Filter
    {
        float fq;
        float qf;
        float gn;
        FilterType type;

        bool hasGain;
        bool hasQ;

        bool implemented;

        static bool typeHasGain(FilterType filterType)
        {
            return create(0, 0, 0, filterType).hasGain;
        }

        static bool typeHasQ(FilterType filterType)
        {
            return create(0, 0, 0, filterType).hasQ;
        }
 
        static bool typeImplemented(FilterType filterType)
        {
            return create(0, 0, 0, filterType).implemented;
        }

        static Filter create(float frequency, float q, float gain, FilterType filtertype)
        {
            Filter filter;
            filter.fq = frequency;
            filter.qf = q;
            filter.gn = gain;
            filter.type = filtertype;
            filter.hasGain = false;
            filter.hasQ = false;
            filter.implemented = true;

            switch (filter.type)
            {
            case BiquadAudioProcessor::SecondOrderLPF:
                filter.hasQ = true;
                break;
            case BiquadAudioProcessor::SecodOrderHPF:
                filter.hasQ = true;
                break;
            case BiquadAudioProcessor::SecondOrderBPF:
                filter.hasQ = true;
                break;
             case BiquadAudioProcessor::FirstOrderHighShelf:
				filter.hasGain = true;
                break;
            case BiquadAudioProcessor::FirstOrderLowShelf:
				filter.hasGain = true;
                break;
            case BiquadAudioProcessor::SecondOrderParametricNonConstQ:
				filter.hasGain = true;
                filter.hasQ = true;
                break;
            case BiquadAudioProcessor::FILTERTYPE_MAX:
				filter.implemented = false;
                break;
            default:
                break;
            }
            return filter;
        }
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

    //FFT
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    enum
    {
        fftOrder = 11,
        fftSize = 1 << fftOrder,
        scopeSize = 512
    };

    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData[scopeSize];
    //FFT
	float responseMagnitude[600] = { 0 };
	float responsePhase[600] = { 0 };

private:
    float samplerate;
    float a0, a1, a2, b0, b1, b2, c0, d0;
    float *x1 = nullptr, *x2 = nullptr, *y1 = nullptr, *y2 = nullptr;

    float responseKeyFrequencies[600];

    juce::AudioParameterFloat *frequency, *q, *gain;
    juce::AudioParameterChoice *filterType;

    Filter mFilter;
 
    void calculateResponse(int sampleRate, int count, const float* frequency, float* magnitude, float* phase);
    void calculateCoefficients(const Filter& filter);
    void calculateCoefficients(float frequency, float q, float gain, FilterType filtertype);
    float linearToDecibels(const float &linearValue);
    float decibelsToLinear(const float &decibelValue);

    // Test from Ear Level engineering https://www.earlevel.com/main/2021/09/02/biquad-calculator-v3/
    std::complex<float> evalCoefs_biquad3(std::vector<float> coefs, float freq);
    void calcDataOptions();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BiquadAudioProcessor)
};
