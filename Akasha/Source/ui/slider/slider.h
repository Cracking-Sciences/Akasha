/*
  ==============================================================================

    slider.h
    Created: 22 Dec 2024 11:15:26pm
    Author:  ric

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Akasha {
	class SliderWithLabel : public juce::Component {
	public:
		SliderWithLabel(const juce::String& labelText) {
			slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
			slider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, textWidth, 20);
			slider.setRange(0.0, 1.0);
			// slider.setNumDecimalPlacesToDisplay(4);
			addAndMakeVisible(slider);

			label.setText(labelText, juce::dontSendNotification);
			label.setJustificationType(juce::Justification::centred);
			label.setMinimumHorizontalScale(0.3f); 
			label.setEditable(true);
			addAndMakeVisible(label);
		}

		juce::Slider& getSlider() { return slider; }
		juce::Label& getLabel() { return label; }

		void resized() override {
			juce::FlexBox flexBox;
			flexBox.flexDirection = juce::FlexBox::Direction::column;
			flexBox.items.add(juce::FlexItem(label).withMinHeight(10.0f).withFlex(1.0f));
			flexBox.items.add(juce::FlexItem(slider).withFlex(sliderRatio));
			flexBox.performLayout(getLocalBounds());
		}

		juce::String getLabelText() {
			return label.getText();
		}

		void setLabelText(const juce::String& newText) {
			label.setText(newText, juce::dontSendNotification);
		}

		float getTextWidth() {
			return textWidth;
		}

		void setTextWidth(float width) {
			textWidth = width;
		}

		void setSliderRatio(float ratio) {
			sliderRatio = ratio;
		}

		void setSliderRange(double min, double max) {
			slider.setRange(min, max);
		}

	private:
		juce::Slider slider;
		juce::Label label;
		float sliderRatio = 5.0f;
		float textWidth = 80.0f;
	};
}