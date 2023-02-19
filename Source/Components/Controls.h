/*
  ==============================================================================

    Controls.h
    Created: 19 Feb 2023 11:51:04am
    Author:  Jeff

  ==============================================================================
*/

#pragma once

#include "../PluginProcessor.h"
#include "../../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class Controls    : public Component
{
public:
    Controls(BiquadAudioProcessor& processor);
    ~Controls();

    void paint (Graphics&) override;
    void resized() override;

private:
    BiquadAudioProcessor& mProcessor;

    Label mLabelFrequency{ "Frequency", "Frq" };
    Slider mSliderFrequency;
    Label mLabelFrequencyDisplay;

    Label mLabelQFactor{ "Q Factor", "Q" };
    Slider mSliderQFactor;
    Label mLabelQFactorDisplay;

    Label mLabelGain{ "Gain", "Gain" };
    Slider mSliderGain;
    Label mLabelGainDisplay;

    Label mLabelFilterType{ "Filter Type", "Filter" };
    Slider mSliderFilterType;
    Label mLabelFilterTypeDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Controls)
};
