/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Style.h"

//==============================================================================
BiquadAudioProcessorEditor::BiquadAudioProcessorEditor (BiquadAudioProcessor& p)
    : AudioProcessorEditor (&p), mProcessor (p), mControls(p), mSpectrum(p)
{
    setLookAndFeel(&mStyle);

    setSize(480, 270);

    addAndMakeVisible(mControls);
    addAndMakeVisible(mSpectrum);
}

BiquadAudioProcessorEditor::~BiquadAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void BiquadAudioProcessorEditor::paint (Graphics& g)
{
    g.setGradientFill(ColourGradient(
        Style::StyleColours::instance().secondary, float(getWidth() / 2), 0,
        Style::StyleColours::instance().primary, float(getWidth() / 2), float(getHeight()), false));
    g.fillAll();
}

void BiquadAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto headerArea = bounds.removeFromTop(58);
    mControls.setBounds(headerArea);
    mSpectrum.setBounds(bounds);
}
