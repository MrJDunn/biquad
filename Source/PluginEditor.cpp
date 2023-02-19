/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BiquadAudioProcessorEditor::BiquadAudioProcessorEditor (BiquadAudioProcessor& p)
    : AudioProcessorEditor (&p), mProcessor (p), mControls(p)
{
    setLookAndFeel(&mStyle);

    setSize(480, 270);

    addAndMakeVisible(mControls);
}

BiquadAudioProcessorEditor::~BiquadAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void BiquadAudioProcessorEditor::paint (Graphics& g)
{
    g.setGradientFill(ColourGradient(
        Colour::fromRGB(33, 33, 33), float(getWidth() / 2), 0,
        Colour::fromRGB(198, 113, 0), float(getWidth() / 2), float(getHeight()), false));
    g.fillAll();
}

void BiquadAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto headerArea = bounds.removeFromTop(58);
    mControls.setBounds(headerArea);
}
