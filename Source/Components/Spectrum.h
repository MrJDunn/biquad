/*
  ==============================================================================

    Spectrum.h
    Created: 20 Feb 2023 9:38:26pm
    Author:  Jeff

  ==============================================================================
*/

#pragma once

#include "../PluginProcessor.h"
#include "../../JuceLibraryCode/JuceHeader.h"

#include <vector>

//==============================================================================
/*
*/
class Spectrum : public Component, public Timer
{
public:
	Spectrum(BiquadAudioProcessor& processor);
    ~Spectrum();

    void paint (Graphics&) override;
    void resized() override;

private:
    BiquadAudioProcessor& mProcessor;

	// Inherited via Timer
	virtual void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spectrum)
};
