/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "kernel/engine/JSEngine.h"
#include "ui/MyLookAndFeel.h"

#include "ui/console/console.h"
#include "ui/macros/macros.h"
#include "ui/code_editor/builtinCodeEditor.h"
#include "ui/code_editor/webCodeEditor.h"

class AkashaAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
	AkashaAudioProcessorEditor(AkashaAudioProcessor& p, juce::AudioProcessorValueTreeState& vts);
	~AkashaAudioProcessorEditor() override;

	void paint(juce::Graphics&) override;
	void resized() override;
	void mouseDown(const juce::MouseEvent& event) override;

	juce::String getCodeString() const {
		return formulaEditorPointer->getText();
	}

	void setCodeString(const juce::String& newText) {
		formulaEditorPointer->setText(newText);
	}

	void setMacroText(const std::array<juce::String, 8>& newText) {
		for (int i = 0; i < 8; ++i) {
			macroSliderGroupPointer->setMacroText(i, newText[i]);
		}
	}

	const std::array<juce::String, 8> getMacroText() {
		std::array<juce::String, 8> result;
		for (int i = 0; i < 8; ++i) {
			result[i] = macroSliderGroupPointer->getMacroText(i);
		}
		return result;
	}

	void compile() {
		formulaEditorPointer->compile();
	}


private:
	AkashaAudioProcessor& audioProcessor;
	juce::AudioProcessorValueTreeState& valueTreeState;

	// code editor.
	// juce::CodeDocument codeDocument;
	// JavascriptTokeniser codeTokeniser;
	// std::unique_ptr<Akasha::builtinFormulaEditor> formulaEditorPointer;
	std::unique_ptr<Akasha::webCodeEditor> formulaEditorPointer;
	// code console.
	std::unique_ptr<Akasha::CodeConsole> codeConsolePointer;
	// macro sliders.
	std::unique_ptr<Akasha::Macros> macroSliderGroupPointer;
	// custom look and feel.
	Akasha::CustomLookAndFeel customLookAndFeel;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AkashaAudioProcessorEditor)
};
