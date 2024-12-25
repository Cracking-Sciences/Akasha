
#pragma once

#include <JuceHeader.h>

namespace Akasha {
	class DraggableNumberBox : public juce::Component {
	public:
		DraggableNumberBox(int minValue, int maxValue, int step = 1)
			:minValue(minValue), maxValue(maxValue), step(step) {
			addAndMakeVisible(numberLabel);
			numberLabel.setJustificationType(juce::Justification::centred);
			numberLabel.setEditable(false);
			setValue(minValue);
			updateLabel();
			numberLabel.setInterceptsMouseClicks(false, false);
			setInterceptsMouseClicks(true, true);
		}

		void resized() override {
			numberLabel.setBounds(getLocalBounds());
		}

		void mouseDown(const juce::MouseEvent& event) override {
			startDragY = event.getPosition().getY();
			initialValueOnDrag = currentValue;
		}

		void mouseDrag(const juce::MouseEvent& event) override {
			int deltaY = startDragY - event.getPosition().getY();
			int increment = deltaY / 10;
			int newValue = initialValueOnDrag + increment * step;
			setValue(newValue);
		}

		void mouseEnter(const juce::MouseEvent& event) override {
			setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
		}

		void mouseExit(const juce::MouseEvent& event) override {
			setMouseCursor(juce::MouseCursor::NormalCursor);
		}

		int getValue() const {
			return currentValue;
		}

		void setValue(int newValue) {
			currentValue = juce::jlimit(minValue, maxValue, newValue);
			updateLabel();
			if (vts_ptr != nullptr && !idToUse.isEmpty()) {
				if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(vts_ptr->getParameter(idToUse))) {
					intParam->setValueNotifyingHost(currentValue);
				}
			}
		}

		void setVTS(juce::AudioProcessorValueTreeState* vts, juce::String idToUse) {
			vts_ptr = vts;
			idToUse = idToUse;
			if (vts_ptr != nullptr && !idToUse.isEmpty()) {
				if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(vts_ptr->getParameter(idToUse))) {
					currentValue = intParam->get();
					updateLabel();
				}
			}
		}

	private:
		void updateLabel() {
			numberLabel.setText(juce::String(currentValue), juce::dontSendNotification);
		}
		juce::Label numberLabel;
		int currentValue = 0;
		int minValue = 0;
		int maxValue = 100;
		int step = 1;
		int startDragY = 0;
		int initialValueOnDrag = 0;
		juce::AudioProcessorValueTreeState* vts_ptr = nullptr;
		juce::String idToUse;
	};
}