/*
  ==============================================================================

	MyLookAndFeel.h
	Created: 12 Nov 2024 1:19:41am
	Author:  ric

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "./assets/fonts/din.h"

namespace Akasha {
	class CustomLookAndFeel : public juce::LookAndFeel_V4 {
	public:
		CustomLookAndFeel() {
			setDefaultSansSerifTypeface(getCustomFont());
			setColour(juce::CodeEditorComponent::backgroundColourId, juce::Colour(0xFF101010));
		}
		static const juce::Typeface::Ptr getCustomFont() {
			static auto typeface = juce::Typeface::createSystemTypefaceFor(din::DIN_ttf, din::DIN_ttfSize);
			return typeface;
		}
		juce::Font getLabelFont(juce::Label& label) override {
			return juce::Font(juce::Font::getDefaultSansSerifFontName(), 20.0f, juce::Font::plain);
		}
		void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
			float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override {
			// ÉèÖÃ¹ìµÀµÄ´ÖÏ¸
			const float thickness = 0.15f; // ¿ØÖÆ¹ìµÀµÄ´ÖÏ¸ (0.0 µ½ 1.0)
			float radius = juce::jmin(width, height) / 2.0f - 4.0f;
			float centerX = x + width * 0.5f;
			float centerY = y + height * 0.5f;
			float rx = centerX - radius;
			float ry = centerY - radius;
			float rw = radius * 2.0f;

			// ¹ìµÀµÄÆðÊ¼ºÍ½áÊø½Ç¶È
			juce::Path track;
			track.addCentredArc(centerX, centerY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

			// »æÖÆ¹ìµÀ
			g.setColour(juce::Colours::grey);
			g.strokePath(track, juce::PathStrokeType(radius * thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

			// »æÖÆÐýÅ¥µÄÖ¸Õë
			float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
			juce::Path pointer;
			float pointerLength = radius * 1.0f;       // ¿ØÖÆÐýÅ¥Ö¸Õë³¤¶È
			float pointerThickness = 4.0f;             // ¿ØÖÆÐýÅ¥Ö¸ÕëµÄ´ÖÏ¸

			pointer.addRoundedRectangle(-pointerThickness * 0.5f, -radius * 1.1f, pointerThickness, pointerLength, 3.0f);
			g.setColour(juce::Colours::orange);
			g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
		}

		void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
			float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override {
			// ¿ØÖÆÏßÐÔ»¬¿é¹ìµÀºÍ»¬¿éÖ¸ÕëµÄ´ÖÏ¸
			float trackThickness = 6.0f; // ÉèÖÃ¹ìµÀ´ÖÏ¸
			float thumbRadius = 10.0f;   // ÉèÖÃ»¬¿éÖ¸Õë´ÖÏ¸

			juce::Rectangle<float> track;
			if (style == juce::Slider::LinearHorizontal)
				track = juce::Rectangle<float>(x, y + (height - trackThickness) * 0.5f, width, trackThickness);
			else if (style == juce::Slider::LinearVertical)
				track = juce::Rectangle<float>(x + (width - trackThickness) * 0.5f, y, trackThickness, height);

			// »æÖÆ¹ìµÀ
			g.setColour(juce::Colours::grey);
			g.fillRect(track);

			// »æÖÆ»¬¿éÖ¸Õë
			g.setColour(juce::Colours::orange);
			juce::Rectangle<float> thumb;
			if (style == juce::Slider::LinearHorizontal)
				thumb = juce::Rectangle<float>(sliderPos - thumbRadius, track.getCentreY() - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f);
			else if (style == juce::Slider::LinearVertical)
				thumb = juce::Rectangle<float>(track.getCentreX() - thumbRadius, sliderPos - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f);

			g.fillEllipse(thumb);
		}

	private:
		juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override {
			return getCustomFont();
		}
	};
}