#include "draggableNumberBox.h"

namespace Akasha {

	void DraggableNumberBoxParameterListener::parameterChanged(const juce::String& parameterID, float newValue) {
		if (parameterID == owner.getIdToUse()) {
			owner.updateCurrentValueFromParameter(newValue);
		}
	}

	DraggableNumberBox::DraggableNumberBox(int minValue, int maxValue, int step)
		:minValue(minValue), maxValue(maxValue), step(step) {
		addAndMakeVisible(numberLabel);
		numberLabel.setJustificationType(juce::Justification::centred);
		numberLabel.setEditable(false);
		setValue(minValue);
		updateLabel();
		numberLabel.setInterceptsMouseClicks(false, false);
		setInterceptsMouseClicks(true, true);
	}

	void DraggableNumberBox::resized() {
		numberLabel.setBounds(getLocalBounds());
	}

	void DraggableNumberBox::mouseDown(const juce::MouseEvent& event) {
		startDragY = event.getPosition().getY();
		initialValueOnDrag = currentValue;
	}

	void DraggableNumberBox::mouseDrag(const juce::MouseEvent& event) {
		int deltaY = startDragY - event.getPosition().getY();
		int increment = deltaY / 10;
		int newValue = initialValueOnDrag + increment * step;
		setValue(newValue);
	}

	void DraggableNumberBox::mouseEnter(const juce::MouseEvent& event) {
		setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
	}

	void DraggableNumberBox::mouseExit(const juce::MouseEvent& event) {
		setMouseCursor(juce::MouseCursor::NormalCursor);
	}

	int DraggableNumberBox::getValue() const {
		return currentValue;
	}

	void DraggableNumberBox::setValue(int newValue) {
		currentValue = juce::jlimit(minValue, maxValue, newValue);
		if (vts_ptr != nullptr && !idToUse.isEmpty()) {
			if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(vts_ptr->getParameter(idToUse))) {
				int start = intParam->getRange().getStart();
				int length = intParam->getRange().getLength();
				float normalizedValue = static_cast<float>(currentValue - start) / length;
				intParam->setValueNotifyingHost(normalizedValue);
			}
		}
	}

	void DraggableNumberBox::setVTS(juce::AudioProcessorValueTreeState* vts, juce::String idToUse) {
		vts_ptr = vts;
		this->idToUse = idToUse;
		parameterListener = std::make_unique<DraggableNumberBoxParameterListener>(*this);
		vts_ptr->addParameterListener(idToUse, parameterListener.get());
		currentValue = vts_ptr->getRawParameterValue(idToUse)->load();
		updateLabel();
	}

	void DraggableNumberBox::updateLabel() {
		numberLabel.setText(juce::String(currentValue), juce::dontSendNotification);
	}

	void DraggableNumberBox::updateCurrentValueFromParameter(float newValue) {
		currentValue = juce::jlimit(minValue, maxValue, static_cast<int>(newValue));
		updateLabel();
	}

	juce::String DraggableNumberBox::getIdToUse() {
		return idToUse;
	}

}