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
#include "ui/oversampling_box/oversamplingBox.h"
#include "ui/draggable_number_box/draggableNumberBox.h"
#include "ui/adsr/adsr.h"

class AkashaAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Button::Listener {
public:
	AkashaAudioProcessorEditor(AkashaAudioProcessor& p, juce::AudioProcessorValueTreeState& vts);
	~AkashaAudioProcessorEditor() override;

	void paint(juce::Graphics&) override;
	void resized() override;

	juce::String getCodeString() const;
	void setCodeString(const juce::String& newText); 
	void setMacroText(const std::array<juce::String, 8>& newText);
	const std::array<juce::String, 8> getMacroText();
	void compile();


	void buttonClicked(juce::Button* button) override;

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
	// oversampling factor.
	std::unique_ptr<Akasha::OversamplingBox> oversamplingBoxPointer;

	// save / load
	std::unique_ptr<juce::TextButton> saveButton;
	std::unique_ptr<juce::TextButton> loadButton;
	std::unique_ptr<juce::FileChooser> fileChooser;
	// view dependency
	std::unique_ptr<juce::TextButton>  viewDependencyButton;

	// adsr
	std::unique_ptr<Akasha::ADSRWidget> adsrWidgetPointer;
	// credit
	std::unique_ptr<juce::TextEditor> creditLabelPointer;

	// custom look and feel.
	Akasha::CustomLookAndFeel customLookAndFeel;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AkashaAudioProcessorEditor)
};
