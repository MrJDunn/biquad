/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Components/Controls.h"
#include "Components/Spectrum.h"
#include "Style.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BiquadAudioProcessorEditor  : /*public GenericAudioProcessorEditor*/ public AudioProcessorEditor
{
public:
    BiquadAudioProcessorEditor (BiquadAudioProcessor&);
    ~BiquadAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BiquadAudioProcessor& mProcessor;

    Controls mControls;
    Spectrum mSpectrum;

    Style mStyle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BiquadAudioProcessorEditor)
};
