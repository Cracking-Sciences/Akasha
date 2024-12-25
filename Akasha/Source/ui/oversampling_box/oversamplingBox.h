
#pragma once

#include <JuceHeader.h>

#include "../draggable_number_box/draggableNumberBox.h"

namespace Akasha {
	class OversamplingBox : public juce::Component {
	public:
		OversamplingBox(juce::String Text, juce::AudioProcessorValueTreeState& vts, juce::String idToUse)
			:box(0, 3, 1), valueTreeState(vts), idToUse(idToUse)
		{
			addAndMakeVisible(label);
			label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
			label.setText(Text, juce::dontSendNotification);
			label.setJustificationType(juce::Justification::centredRight);
			addAndMakeVisible(box);
			box.setVTS(&valueTreeState, idToUse);
		}

		void resized() override {
			juce::Rectangle<int> bounds = getLocalBounds();
			juce::FlexBox flexBox;
			flexBox.flexDirection = juce::FlexBox::Direction::row;
			flexBox.justifyContent = juce::FlexBox::JustifyContent::flexEnd;
			flexBox.items.add(juce::FlexItem(label).withHeight(25.0f).withMinWidth(160.0f));
			flexBox.items.add(juce::FlexItem(box).withHeight(25.0f).withMinWidth(25.0f));
			flexBox.performLayout(getLocalBounds());
		}

		int getValue() const {
			return box.getValue();
		}

		void setValue(int newValue) {
			box.setValue(newValue);
		}

	private:
		juce::AudioProcessorValueTreeState& valueTreeState;
		juce::String idToUse;
		juce::Label label;
		DraggableNumberBox box;
	};
}