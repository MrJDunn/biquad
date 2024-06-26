/*
  ==============================================================================

    Spectrum.cpp
    Created: 20 Feb 2023 9:38:26pm
    Author:  Jeff

  ==============================================================================
*/

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Spectrum.h"
#include "../Style.h"

//==============================================================================
Spectrum::Spectrum(BiquadAudioProcessor& processor)
	: mProcessor{ processor }
{
    startTimer(60);
}

Spectrum::~Spectrum()
{
}

void Spectrum::paint(Graphics& g)
{
	float width = (float)getWidth();
	float height = (float)getHeight();

	float barWidth = 0.75f;

	auto sampleGradient = ColourGradient(
		Style::StyleColours::instance().primaryDark, width / 2.f, 0,
		Style::StyleColours::instance().primaryLight, width / 2.f, height, false);

	for (int i = 0; i < mProcessor.scopeSize; ++i)
	{
		g.setGradientFill(sampleGradient);
		g.fillRect((float)i, height - mProcessor.scopeData[i] * height, barWidth, height);
	}
	g.setColour(Colours::black);

	for (int i = 0; i < 512; ++i)
	{
		g.fillRect((float)i, height - mProcessor.responseMagnitude[i] * height, barWidth, height);
	}
}

void Spectrum::resized()
{
}

void Spectrum::timerCallback()
{
	static Random rand;

	if (mProcessor.nextFFTBlockReady)
	{
		mProcessor.window.multiplyWithWindowingTable(mProcessor.fftData, mProcessor.fftSize);
		mProcessor.forwardFFT.performFrequencyOnlyForwardTransform(mProcessor.fftData);

		auto mindB = -100.0f;
		auto maxdB = 0.0f;

		for (int i = 0; i < mProcessor.scopeSize; ++i)
		{
			auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)mProcessor.scopeSize) * 0.8f);
			auto fftDataIndex = juce::jlimit(0, mProcessor.fftSize / 2, (int)(skewedProportionX * (float)mProcessor.fftSize * 0.5f));
			auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(mProcessor.fftData[fftDataIndex])
				- juce::Decibels::gainToDecibels((float)mProcessor.fftSize)), mindB, maxdB, 0.0f, 1.0f);

			mProcessor.scopeData[i] = level;
		}
		mProcessor.nextFFTBlockReady = false;
		repaint();
	}
}
