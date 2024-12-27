
#pragma once

#include <JuceHeader.h>

namespace Akasha {
	class DraggableNumberBox : public juce::Component
, private juce::AudioProcessorValueTreeState::Listener{
	public:
		DraggableNumberBox(int minValue, int maxValue, int step = 1);

		~DraggableNumberBox();

		void resized() override;

		void mouseDown(const juce::MouseEvent& event) override;

		void mouseDrag(const juce::MouseEvent& event) override;

		void mouseEnter(const juce::MouseEvent& event) override;

		void mouseExit(const juce::MouseEvent& event) override;

		void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

		int getValue() const;

		void setValue(int newValue);

		void setVTS(juce::AudioProcessorValueTreeState* vts, juce::String idToUse);

	private:
		void updateLabel();

		void parameterChanged(const juce::String& parameterID, float newValue) override;

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