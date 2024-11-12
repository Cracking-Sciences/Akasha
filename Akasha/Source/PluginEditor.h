/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "./assets/fonts/din.h"

//==============================================================================
/**
*/

namespace Akasha {

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
	CustomLookAndFeel() {
		setDefaultSansSerifTypeface(getCustomFont());
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
		// ���ù���Ĵ�ϸ
		const float thickness = 0.15f; // ���ƹ���Ĵ�ϸ (0.0 �� 1.0)
		float radius = juce::jmin(width, height) / 2.0f - 4.0f;
		float centerX = x + width * 0.5f;
		float centerY = y + height * 0.5f;
		float rx = centerX - radius;
		float ry = centerY - radius;
		float rw = radius * 2.0f;

		// �������ʼ�ͽ����Ƕ�
		juce::Path track;
		track.addCentredArc(centerX, centerY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

		// ���ƹ��
		g.setColour(juce::Colours::grey);
		g.strokePath(track, juce::PathStrokeType(radius * thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

		// ������ť��ָ��
		float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
		juce::Path pointer;
		float pointerLength = radius * 1.0f;       // ������ťָ�볤��
		float pointerThickness = 4.0f;             // ������ťָ��Ĵ�ϸ

		pointer.addRoundedRectangle(-pointerThickness * 0.5f, -radius*1.1f, pointerThickness, pointerLength, 3.0f);
		g.setColour(juce::Colours::orange);
		g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
	}

	void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
		float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override {
		// �������Ի������ͻ���ָ��Ĵ�ϸ
		float trackThickness = 6.0f; // ���ù����ϸ
		float thumbRadius = 10.0f;   // ���û���ָ���ϸ

		juce::Rectangle<float> track;
		if (style == juce::Slider::LinearHorizontal)
			track = juce::Rectangle<float>(x, y + (height - trackThickness) * 0.5f, width, trackThickness);
		else if (style == juce::Slider::LinearVertical)
			track = juce::Rectangle<float>(x + (width - trackThickness) * 0.5f, y, trackThickness, height);

		// ���ƹ��
		g.setColour(juce::Colours::grey);
		g.fillRect(track);

		// ���ƻ���ָ��
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


class SliderWithLabel : public juce::Component {
public:
	SliderWithLabel(const juce::String& labelText)
	{
		slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
		slider.setRange(0.0, 1.0);
		slider.setNumDecimalPlacesToDisplay(3);
		addAndMakeVisible(slider);

		label.setText(labelText, juce::dontSendNotification);
		label.setJustificationType(juce::Justification::centred);
		label.setEditable(false);
		addAndMakeVisible(label);
	}

	juce::Slider& getSlider() { return slider; }
	juce::Label& getLabel() { return label; }

	void resized() override
	{
		juce::FlexBox flexBox;
		flexBox.flexDirection = juce::FlexBox::Direction::column;
		flexBox.items.add(juce::FlexItem(label).withFlex(1.0f));
		flexBox.items.add(juce::FlexItem(slider).withFlex(5.0f));
		flexBox.performLayout(getLocalBounds());
	}

private:
	juce::Slider slider;
	juce::Label label;
};

class FormulaEditor : public juce::TextEditor {
public:
	FormulaEditor() :juce::TextEditor("FormulaEditor") {
		setMultiLine(true, false);
		setScrollbarsShown(true);
		setTabKeyUsedAsCharacter(true);
		setReturnKeyStartsNewLine(true);
		setScrollbarsShown(true);
		setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::plain));
	}

	bool keyPressed(const juce::KeyPress& key) {
		if (key == juce::KeyPress(juce::KeyPress::returnKey, juce::ModifierKeys::shiftModifier, NULL)) {
			if (console != nullptr) {
				console->setText(getText());
			}
			return true;
		}
		return juce::TextEditor::keyPressed(key);
	}

	void setConsole(juce::TextEditor* console) {
		this->console = console;
	}

private:
	juce::TextEditor* console = nullptr;
};


}
class AkashaAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
	AkashaAudioProcessorEditor(AkashaAudioProcessor&);
	~AkashaAudioProcessorEditor() override;

	//==============================================================================
	void paint(juce::Graphics&) override;
	void resized() override;

private:
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	AkashaAudioProcessor& audioProcessor;
	juce::OwnedArray<Akasha::SliderWithLabel> macroSliders;  // macro sliders.
	Akasha::FormulaEditor formulaEditor;    // code editor.
	juce::TextEditor code_console;       // console.
	Akasha::CustomLookAndFeel customLookAndFeel;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AkashaAudioProcessorEditor)
};
