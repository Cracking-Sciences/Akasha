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
			slider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 80, 20);
			slider.setRange(0.0, 1.0);
			// slider.setNumDecimalPlacesToDisplay(4);
			addAndMakeVisible(slider);
			label.setText(labelText, juce::dontSendNotification);
			label.setJustificationType(juce::Justification::centred);
			label.setMinimumHorizontalScale(0.3f); 
			label.setEditable(true);
			label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
			addAndMakeVisible(label);
		}

		juce::Slider& getSlider() { return slider; }
		juce::Label& getLabel() { return label; }

		void resized() override {
			juce::FlexBox flexBox;
			float width = getLocalBounds().getWidth();
			slider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, width, 20);
			flexBox.flexDirection = juce::FlexBox::Direction::column;
			flexBox.items.add(juce::FlexItem(label).withMinHeight(20.0f).withMinWidth(width));
			flexBox.items.add(juce::FlexItem(slider).withMinWidth(width).withFlex(1.0));
			flexBox.performLayout(getLocalBounds());
		}

		juce::String getLabelText() {
			return label.getText();
		}

		void setLabelText(const juce::String& newText) {
			label.setText(newText, juce::dontSendNotification);
		}

	private:
		juce::Slider slider;
		juce::Label label;
	};
}