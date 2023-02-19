/*
  ==============================================================================

    Controls.cpp
    Created: 19 Feb 2023 11:51:04am
    Author:  Jeff

  ==============================================================================
*/

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Controls.h"

//==============================================================================
Controls::Controls(BiquadAudioProcessor& processor) : mProcessor{ processor }
{
	setOpaque(false);

	auto boldFont = Font(8.f, 1);
	auto notBoldFont = Font(8.f, 0);
	auto smallerFont = Font(7.f, 0);

	addAndMakeVisible(mLabelFrequency);
	mLabelFrequency.setFont(boldFont);
	mLabelFrequency.setJustificationType(Justification::centred);

	addAndMakeVisible(mLabelQFactor);
	mLabelQFactor.setFont(boldFont);
	mLabelQFactor.setJustificationType(Justification::centred);

	addAndMakeVisible(mLabelGain);
	mLabelGain.setFont(boldFont);
	mLabelGain.setJustificationType(Justification::centred);

	addAndMakeVisible(mLabelFilterType);
	mLabelFilterType.setFont(boldFont);
	mLabelFilterType.setJustificationType(Justification::centred);


	addAndMakeVisible(mLabelFrequencyDisplay);
	mLabelFrequencyDisplay.setFont(smallerFont);
	mLabelFrequencyDisplay.setJustificationType(Justification::centred);

	addAndMakeVisible(mLabelQFactorDisplay);
	mLabelQFactorDisplay.setFont(smallerFont);
	mLabelQFactorDisplay.setJustificationType(Justification::centred);

	addAndMakeVisible(mLabelGainDisplay);
	mLabelGainDisplay.setFont(boldFont);
	mLabelGainDisplay.setJustificationType(Justification::centred);

	addAndMakeVisible(mLabelFilterTypeDisplay);
	mLabelFilterTypeDisplay.setFont(boldFont);
	mLabelFilterTypeDisplay.setJustificationType(Justification::centred);


	addAndMakeVisible(mSliderFrequency);
	mSliderFrequency.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
	mSliderFrequency.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 30, 10);
	mSliderFrequency.setRange(20.f, 20000.f, 1.f);
	mSliderFrequency.onValueChange = [this]
	{
		float val = mSliderFrequency.getValue() / (20000.f - 20.f);
		mProcessor.setParameterNotifyingHost(BiquadAudioProcessor::Params::FREQUENCY, val);
		mLabelFrequencyDisplay.setText(String(mSliderFrequency.getValue()), dontSendNotification);
	};

	addAndMakeVisible(mSliderQFactor);
	mSliderQFactor.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
	mSliderQFactor.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 30, 10);
	mSliderQFactor.setRange(0.01f, 10.f, 0.01f);
	mSliderQFactor.onValueChange = [this]
	{
		float val = mSliderQFactor.getValue() / (10.f - 0.01f);
		mProcessor.setParameterNotifyingHost(BiquadAudioProcessor::Params::Q, val);
		mLabelQFactorDisplay.setText(String(mSliderQFactor.getValue()), dontSendNotification);
	};

	addAndMakeVisible(mSliderGain);
	mSliderGain.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
	mSliderGain.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 30, 10);
	mSliderGain.setRange(-10.f, 10.f, 0.1f);
	mSliderGain.onValueChange = [this]
	{
		float val = (mSliderGain.getValue() + 10.f) / 20.f;
		mProcessor.setParameterNotifyingHost(BiquadAudioProcessor::Params::GAIN, val);
		mLabelGainDisplay.setText(String(mSliderGain.getValue()), dontSendNotification);
	};

	addAndMakeVisible(mSliderFilterType);
	mSliderFilterType.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
	mSliderFilterType.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 30, 10);
	mSliderFilterType.setRange(0, 23, 1);
	mSliderFilterType.onValueChange = [this]
	{
		float val = mSliderFilterType.getValue() / (float)(BiquadAudioProcessor::FilterType::FILTERTYPE_MAX - 1);
		String txt = mProcessor.getFilterTypeStrings()[mSliderFilterType.getValue()];
		mProcessor.setParameterNotifyingHost(BiquadAudioProcessor::Params::FILTERTYPE, val);
		mLabelFilterTypeDisplay.setText(txt, dontSendNotification);
	};


	// Initialise buttons
	mSliderFrequency.setValue(mProcessor.getParameter(BiquadAudioProcessor::Params::FREQUENCY) * (20000.f - 20.f));
	mSliderQFactor.setValue(mProcessor.getParameter(BiquadAudioProcessor::Params::Q) * (10.f - 0.01f));
	mSliderGain.setValue((mProcessor.getParameter(BiquadAudioProcessor::Params::GAIN) * 20.f) - 10.f);
	mSliderFilterType.setValue(mProcessor.getParameter(BiquadAudioProcessor::Params::FILTERTYPE) * (float)(BiquadAudioProcessor::FilterType::FILTERTYPE_MAX - 1));
}

Controls::~Controls()
{
}

void Controls::paint (Graphics& g)
{
	auto bounds = getLocalBounds();

	g.setColour(Colour::fromRGB(33, 33, 33));
	// top bar
	g.fillRect(0, 0, getWidth(), 24);

	// center rounded rectangle
	g.fillRoundedRectangle(getWidth() / 2 - 246 / 2, 0, 246, getHeight(), 24.0f);

	g.setColour(Colours::white);
	g.setFont(Font(12.5f, 1));
	g.drawFittedText("BIQUAD", { 8,4,getWidth(),16 }, Justification::left, 1);
}

void Controls::resized()
{
	auto bounds = getLocalBounds();

	bounds.removeFromLeft(getWidth() / 2 - 246 / 2);

	auto labelHeight = 10;
	auto width = 54;
	auto dialSize = 35;

	auto frequencyBounds = bounds.removeFromLeft(width);
	mLabelFrequency.setBounds(frequencyBounds.removeFromTop(labelHeight));
	mSliderFrequency.setBounds(frequencyBounds.removeFromTop(dialSize));
	mLabelFrequencyDisplay.setBounds(frequencyBounds);

	auto qBounds = bounds.removeFromLeft(width);
	mLabelQFactor.setBounds(qBounds.removeFromTop(labelHeight));
	mSliderQFactor.setBounds(qBounds.removeFromTop(dialSize));
	mLabelQFactorDisplay.setBounds(qBounds);

	auto gainBounds = bounds.removeFromLeft(width);
	mLabelGain.setBounds(gainBounds.removeFromTop(labelHeight));
	mSliderGain.setBounds(gainBounds.removeFromTop(dialSize));
	mLabelGainDisplay.setBounds(gainBounds);

	auto filterTypeBounds = bounds.removeFromLeft(width);
	mLabelFilterType.setBounds(filterTypeBounds.removeFromTop(labelHeight));
	mSliderFilterType.setBounds(filterTypeBounds.removeFromTop(dialSize));
	mLabelFilterTypeDisplay.setBounds(filterTypeBounds);
}
