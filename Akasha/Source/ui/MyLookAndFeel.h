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
		CustomLookAndFeel()  : LookAndFeel_V4() {
			setDefaultSansSerifTypeface(getCustomFont());
			setColour(juce::CodeEditorComponent::backgroundColourId, juce::Colour(0xFF252525));
			setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF353535));
			setColour(juce::TextEditor::outlineColourId, outlineColour);
			setColour(juce::Label::backgroundColourId, juce::Colour(0xFF303030));
			setColour(juce::Label::outlineColourId, outlineColour);
			setColour(juce::Label::backgroundWhenEditingColourId, juce::Colour(0xFF303030));
			setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF303030));

			setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF303030));
			setColour(juce::DocumentWindow::backgroundColourId, juce::Colour(0xFF303030));
			setColour(juce::ListBox::backgroundColourId, juce::Colour(0xFF303030));
			setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF303030));
			setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF303030));

			setColour(juce::TextButton::buttonColourId, outlineColour);
			setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF505050));
			setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF202020));
			setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF202020));

			setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xFF303030));
		}

		static const juce::Typeface::Ptr getCustomFont() {
			static auto typeface = juce::Typeface::createSystemTypefaceFor(din::DIN_ttf, din::DIN_ttfSize);
			return typeface;
		}

		void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
			float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override {
			const float thickness = 0.15f;
			float radius = juce::jmin(width, height) / 2.0f - 4.0f;
			float centerX = x + width * 0.5f;
			float centerY = y + height * 0.5f;
			float rx = centerX - radius;
			float ry = centerY - radius;
			float rw = radius * 2.0f;

			juce::Path track;
			track.addCentredArc(centerX, centerY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

			if (slider.isMouseOver()|| slider.isMouseButtonDown()) {
				g.setColour(juce::Colours::lightgrey);
			} else {
				g.setColour(juce::Colours::grey);
			}
			g.strokePath(track, juce::PathStrokeType(radius * thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

			float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
			juce::Path pointer;
			float pointerLength = radius * 1.0f;
			float pointerThickness = 4.0f;

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

		void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
			auto bounds = juce::Rectangle<int>(0, 0, width, height);
			drawCoolBorder(g, bounds, outlineColour, 2, 7);
		}

		void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
			auto bounds = juce::Rectangle<int>(0, 0, width, height);
			drawCoolBackground(g, bounds,textEditor.findColour(juce::TextEditor::backgroundColourId), 7);
		}

		void drawLabel(juce::Graphics& g, juce::Label& label) override { 
			if (label.isBeingEdited()) {
				return;
			}
			drawCoolBackground(g, label.getLocalBounds(),label.findColour(juce::Label::backgroundColourId), 7);
			g.setColour(label.findColour(juce::Label::textColourId));
			g.setFont(label.getFont().withHeight(20.0f));
			g.drawFittedText(label.getText(), label.getLocalBounds().reduced(4, 0), label.getJustificationType(), 1);
			drawCoolBorder(g, label.getLocalBounds(), outlineColour, 2, 7);
		}

		void drawButtonBackground(juce::Graphics& g, juce::Button& button,
			const juce::Colour& backgroundColour,
			bool isMouseOverButton, bool isButtonDown) override
		{
			auto bounds = button.getLocalBounds();

			auto pressedColour = button.findColour(juce::TextButton::buttonOnColourId);

			auto colour = isButtonDown ? pressedColour 
				: (isMouseOverButton ? backgroundColour.brighter(0.2f) : backgroundColour);
			drawCoolBackground(g, bounds, colour, 7);
			drawCoolBorder(g, bounds, outlineColour, 2, 7);
		}

	private:
		juce::Typeface::Ptr getTypefaceForFont(const juce::Font& f) override {
			return getCustomFont();
		}

		void drawCoolBorder(juce::Graphics& g, const juce::Rectangle<int>& bounds, juce::Colour borderColour, float borderWidth, float cutDistance) {
			int x1 = bounds.getRight();
			int y1 = bounds.getBottom();

			juce::Path borderPath;
			borderPath.startNewSubPath(x1, bounds.getY());
			borderPath.lineTo(x1, y1 - cutDistance);

			borderPath.lineTo(x1 - cutDistance, y1);
			borderPath.lineTo(bounds.getX(), y1);

			g.setColour(borderColour);
			g.strokePath(borderPath, juce::PathStrokeType((float)borderWidth));
		}

		void drawCoolBackground(juce::Graphics& g, const juce::Rectangle<int>& bounds, juce::Colour backgroundColour, int cutDistance) {
			juce::Path clipPath;
			int x1 = bounds.getRight();
			int y1 = bounds.getBottom();

			clipPath.startNewSubPath(bounds.getX(), bounds.getY());
			clipPath.lineTo(x1 - cutDistance, bounds.getY());  // 右上角
			clipPath.lineTo(x1, bounds.getY());               // 右侧上段
			clipPath.lineTo(x1, y1 - cutDistance);            // 右侧下段
			clipPath.lineTo(x1 - cutDistance, y1);            // 下侧右段
			clipPath.lineTo(bounds.getX(), y1);               // 下侧左段
			clipPath.closeSubPath();
			g.setColour(backgroundColour);
			g.fillPath(clipPath);
		}

		juce::Colour outlineColour = juce::Colour(0xFFA0A0A0);
	};
}