/*
  ==============================================================================

    Spectrum.cpp
    Created: 20 Feb 2023 9:38:26pm
    Author:  Jeff

  ==============================================================================
*/

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Spectrum.h"

//==============================================================================
Spectrum::Spectrum(BiquadAudioProcessor& processor)
	: mProcessor{ processor }
{
    startTimer(60);
}

Spectrum::~Spectrum()
{
}

void Spectrum::paint (Graphics& g)
{
	for (int i = 1; i < mProcessor.scopeSize; ++i)
	{
		auto width = getLocalBounds().getWidth();
		auto height = getLocalBounds().getHeight();

		g.drawLine({ (float)juce::jmap(i - 1, 0, mProcessor.scopeSize - 1, 0, width),
							  juce::jmap(mProcessor.scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
					  (float)juce::jmap(i,     0, mProcessor.scopeSize - 1, 0, width),
							  juce::jmap(mProcessor.scopeData[i],     0.0f, 1.0f, (float)height, 0.0f) });
	}
	/*
	int i = 0;
	float width = (float)getWidth();
	float height = (float)getHeight();

	float barWidth = 0.5f;

	auto sampleGradient = ColourGradient(
		Colour::fromRGB(33, 33, 33), width / 2.f, -58,
		Colour::fromRGB(198, 113, 0), width / 2.f, height, false);

	auto interpolatedGradient = ColourGradient(
		Colour::fromRGB(33, 33, 33), width / 2.f, -58,
		Colour::fromRGB(255, 160, 0), width / 2.f, height, false);

	for (auto it : fifo)
	{
		g.setGradientFill(interpolatedGradient);
		g.fillRect((float)i, height - it * height, barWidth, height);
		i += 1;
	}
	*/
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
			auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)mProcessor.scopeSize) * 0.2f);
			auto fftDataIndex = juce::jlimit(0, mProcessor.fftSize / 2, (int)(skewedProportionX * (float)mProcessor.fftSize * 0.5f));
			auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(mProcessor.fftData[fftDataIndex])
				- juce::Decibels::gainToDecibels((float)mProcessor.fftSize)), mindB, maxdB, 0.0f, 1.0f);

			mProcessor.scopeData[i] = level;
		}
		mProcessor.nextFFTBlockReady = false;
		repaint();
	}
}
