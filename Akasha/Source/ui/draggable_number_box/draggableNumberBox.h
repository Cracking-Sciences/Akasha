
#pragma once

#include <JuceHeader.h>

namespace Akasha {
	class DraggableNumberBox;

	class DraggableNumberBoxParameterListener : public juce::AudioProcessorValueTreeState::Listener {
	public:
		DraggableNumberBoxParameterListener(DraggableNumberBox& owner) : owner(owner) {}
		void parameterChanged(const juce::String& parameterID, float newValue) override;
	private:
		DraggableNumberBox& owner;
	};

	class DraggableNumberBox : public juce::Component {
	public:
		DraggableNumberBox(int minValue, int maxValue, int step = 1);

		void resized() override;

		void mouseDown(const juce::MouseEvent& event) override;

		void mouseDrag(const juce::MouseEvent& event) override;

		void mouseEnter(const juce::MouseEvent& event) override;

		void mouseExit(const juce::MouseEvent& event) override;

		int getValue() const;

		void setValue(int newValue);

		void setVTS(juce::AudioProcessorValueTreeState* vts, juce::String idToUse);

		void updateCurrentValueFromParameter(float normalizedValue);

		juce::String getIdToUse();

	private:
		void updateLabel();

		juce::Label numberLabel;
		int currentValue = 0;
		int minValue = 0;
		int maxValue = 100;
		int step = 1;
		int startDragY = 0;
		int initialValueOnDrag = 0;
		juce::AudioProcessorValueTreeState* vts_ptr = nullptr;
		juce::String idToUse;
		std::unique_ptr<DraggableNumberBoxParameterListener> parameterListener;
	};
}