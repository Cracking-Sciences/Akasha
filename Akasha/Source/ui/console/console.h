
#pragma once
#include <JuceHeader.h>

namespace Akasha{
	class CodeConsole: public juce::TextEditor {
	public:
		CodeConsole() {}
	
		void paint(juce::Graphics& g) override {
			juce::TextEditor::paint(g);
	
			auto outlineColour = findColour(juce::TextEditor::outlineColourId);
			g.setColour(outlineColour);
			g.drawLine(0.0f, 0.0f, static_cast<float>(getWidth()), 0.0f, 2.0f);
		}
	};
}