/*
  ==============================================================================

    Style.h
    Created: 19 Feb 2023 12:41:36pm
    Author:  Jeff

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Style : public LookAndFeel_V4
{
public:
	Style() 
	{
		setColour(Slider::ColourIds::textBoxOutlineColourId, colours.invisible);
		setColour(Slider::ColourIds::textBoxHighlightColourId, colours.invisible);
		setColour(Slider::ColourIds::textBoxHighlightColourId, colours.invisible);
		setColour(Label::ColourIds::outlineColourId, colours.invisible);

		setColour(TextEditor::ColourIds::outlineColourId, colours.invisible);
	};

	~Style() {};

	// For toggle buttons
	void drawTickBox(Graphics& g, Component& /*component*/,
		float x, float y, float w, float h,
		const bool ticked,
		const bool isEnabled,
		const bool shouldDrawButtonAsHighlighted,
		const bool shouldDrawButtonAsDown) override
	{
		ignoreUnused(isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

		auto diameter = std::min(w, h);
		auto radius = diameter / 2.f;

		auto bounds = juce::Rectangle<int>(x + w / 2 - radius, y + h / 2 - radius, diameter, diameter).toFloat();

		if (ticked)
		{
			g.setGradientFill(ColourGradient(
				colours.tertiaryLight, bounds.getCentreX(), bounds.getCentreY(),
				colours.secondaryLight, bounds.getWidth() * 2, bounds.getHeight() * 2, true));

			auto ellipseSpace = bounds.reduced(2);
			g.fillEllipse(ellipseSpace);

			g.setColour(colours.primary);
			g.drawEllipse(bounds, 1.f);
		}
		else
		{
			g.setColour(colours.secondary);

			auto ellipseSpace = bounds.reduced(2);
			g.fillEllipse(ellipseSpace);

			g.setColour(colours.secondaryLight);
			g.drawEllipse(bounds, 1.f);
		}
	} // drawTickBox

	void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
		const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
	{
		auto diameter = std::min(width, height);
		auto radius = diameter / 2.f;

		auto bounds = juce::Rectangle<int>(x + width / 2 - radius, y + height / 2 - radius, diameter, diameter).toFloat();

		auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
		auto lineW = 1.0f;// jmin(8.0f, radius * 0.5f);
		auto arcRadius = radius - lineW * 0.5f;

		g.setGradientFill(ColourGradient(
		    slider.isEnabled() ? colours.tertiaryLight : colours.grey, bounds.getCentreX(), bounds.getCentreY(),
			colours.secondaryLight, bounds.getCentreX() + 2 * diameter, bounds.getCentreY() + 2 * diameter, true));
		g.fillEllipse(bounds.reduced(4));

		Path valueArc;
		valueArc.addCentredArc(bounds.getCentreX(),
			bounds.getCentreY(),
			arcRadius,
			arcRadius,
			0.0f,
			rotaryStartAngle,
			toAngle,
			true);

		g.setColour(slider.isEnabled() ? colours.primary : colours.secondaryLight);
		g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));

		auto thumbWidth = lineW * 5.0f;
		Point<float> thumbPoint(bounds.getCentreX() + (arcRadius - 7) * std::cos(toAngle - MathConstants<float>::halfPi),
			bounds.getCentreY() + (arcRadius - 7) * std::sin(toAngle - MathConstants<float>::halfPi));

		g.setColour(slider.isEnabled() ? colours.white : colours.secondary);
		g.fillEllipse(juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));


	} // drawRotarySlider

	void drawButtonBackground(Graphics& g,
		Button& button,
		const Colour& /*backgroundColour*/,
		bool /*shouldDrawButtonAsHighlighted*/,
		bool /*shouldDrawButtonAsDown*/) override
	{
		auto cornerSize = 5.0f;
		juce::Rectangle<int> boxBounds(0, 0, button.getWidth(), button.getHeight());

		g.setGradientFill(ColourGradient(
			button.getLookAndFeel().findColour(MidiKeyboardComponent::ColourIds::keySeparatorLineColourId),
			(float)button.getWidth(), (float)button.getHeight(),
			colours.secondary,
			(float)button.getWidth(), (float)button.getHeight() / 2.0f, false));
		g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

		g.setColour(colours.secondary);
		g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 0.25f);
	} // drawButtonBackground

	void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& textEditor) override
	{
		if (dynamic_cast<AlertWindow*> (textEditor.getParentComponent()) != nullptr)
		{
			auto cornerSize = 5.0f;
			juce::Rectangle<int> boxBounds(0, 0, width, height);

			g.setGradientFill(ColourGradient(
				findColour(MidiKeyboardComponent::ColourIds::keySeparatorLineColourId),
				(float)width, (float)height,
				Colour::fromRGB(17, 17, 17),
				(float)width, (float)height / 2.0f, false));
			g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

			g.setColour(Colour::fromRGB(15, 15, 15));
			g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 0.25f);

			g.setColour(textEditor.findColour(TextEditor::outlineColourId));
			g.drawHorizontalLine(height - 1, 0.0f, static_cast<float> (width));
		}
		else
		{
			auto cornerSize = 5.0f;
			juce::Rectangle<int> boxBounds(0, 0, width, height);

			g.setGradientFill(ColourGradient(
				findColour(MidiKeyboardComponent::ColourIds::keySeparatorLineColourId),
				(float)width, (float)height,
				Colour::fromRGB(17, 17, 17),
				(float)width, (float)height / 2.0f, false));
			g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

			g.setColour(Colour::fromRGB(15, 15, 15));
			g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 0.25f);
		}
	} // fillTextEditorBackground

	void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& textEditor) override
	{
	}

	Label* createSliderTextBox(Slider& slider) override
	{
		auto l = new Label();// SliderLabelComp();

		l->setJustificationType(Justification::centred);
		l->setKeyboardType(TextInputTarget::decimalKeyboard);

		l->setColour(Label::textColourId, slider.findColour(Slider::textBoxTextColourId));
		l->setColour(Label::backgroundColourId,
			(slider.getSliderStyle() == Slider::LinearBar || slider.getSliderStyle() == Slider::LinearBarVertical)
			? Colours::transparentBlack
			: slider.findColour(Slider::textBoxBackgroundColourId));
		l->setColour(Label::outlineColourId, slider.findColour(Slider::textBoxOutlineColourId));
		l->setColour(TextEditor::textColourId, slider.findColour(Slider::textBoxTextColourId));
		l->setColour(TextEditor::backgroundColourId,
			slider.findColour(Slider::textBoxBackgroundColourId)
			.withAlpha((slider.getSliderStyle() == Slider::LinearBar || slider.getSliderStyle() == Slider::LinearBarVertical)
				? 0.7f : 1.0f));
		l->setColour(TextEditor::outlineColourId, slider.findColour(Slider::textBoxOutlineColourId));
		l->setColour(TextEditor::highlightColourId, slider.findColour(Slider::textBoxHighlightColourId));

		return l;
	}

	struct StyleColours
	{
		static StyleColours instance() { return StyleColours(); };

		// Blues
		Colour primary		= Colour::fromRGB(21, 101, 192);
		Colour primaryLight = Colour::fromRGB(94, 146, 243);
		Colour primaryDark	= Colour::fromRGB(0, 60, 143);

		// Blacks
		Colour secondary	  = Colour::fromRGB(33, 33, 33);
		Colour secondaryLight = Colour::fromRGB(72, 72, 72);
		Colour secondaryDark  = Colour::fromRGB(0, 0, 0);

		// Yellows
		Colour tertiary		 = Colour::fromRGB(255, 162, 0);
		Colour tertiaryLight = Colour::fromRGB(255, 210, 73);
		Colour tertiaryDark	 = Colour::fromRGB(198, 112, 0);

		// Utilitity
		Colour invisible = Colours::transparentBlack;
		Colour white = Colours::white;
		Colour black = Colours::black;
		Colour grey = Colour::fromRGB(100, 100, 100);
	} colours;
};
