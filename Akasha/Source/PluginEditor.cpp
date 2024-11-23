/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "JSEngine.h"



AkashaAudioProcessorEditor::AkashaAudioProcessorEditor(AkashaAudioProcessor& p, juce::AudioProcessorValueTreeState& vts):
	AudioProcessorEditor(&p),
	valueTreeState (vts),
	audioProcessor(p),
	codeDocument(),
	codeTokeniser(),
	formulaEditorPointer(std::make_unique<Akasha::FormulaEditor>(codeDocument, &codeTokeniser, audioProcessor.getJSEngine())),
	formulaEditor(*formulaEditorPointer) {
	setLookAndFeel(&customLookAndFeel);

	// several sliders.
	for (int i = 0; i < 8; ++i) {
		auto* sliderWithLabel = new Akasha::SliderWithLabel("m" + juce::String(i));
		addAndMakeVisible(sliderWithLabel);
		macroSliders.add(sliderWithLabel);

		auto* attachment = new SliderAttachment(
				valueTreeState,
				"macro" + juce::String(i),
				sliderWithLabel->getSlider()
			);
		macroSliderAttachments.add(attachment);
	}

	// code editor.
	formulaEditor.setConsole(&code_console);
	addAndMakeVisible(formulaEditor);

	// console.
	code_console.setMultiLine(true);
	code_console.setReadOnly(true);
	code_console.setReturnKeyStartsNewLine(true);
	code_console.setScrollbarsShown(true);
	code_console.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::plain));
	addAndMakeVisible(code_console);

	// main editor.
	setSize(800, 600);
	setResizable(true, true);
	setResizeLimits(600, 400, std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

	// processor
	for (auto voice_ptr : audioProcessor.getVoices()) {
		voice_ptr->setConsole(&code_console);
	}
	setCodeString(audioProcessor.savedCode);    
	setMacroText(audioProcessor.savedMacroText);
}

AkashaAudioProcessorEditor::~AkashaAudioProcessorEditor() {
	setLookAndFeel(nullptr);
}

void AkashaAudioProcessorEditor::paint(juce::Graphics& g) {
	g.fillAll(juce::Colours::darkgrey);
}

void AkashaAudioProcessorEditor::resized() {
	juce::FlexBox mainFlexBox;
	juce::FlexBox macroSlidersBox;
	juce::FlexBox textEditorBox;

	mainFlexBox.flexDirection = juce::FlexBox::Direction::column;

	textEditorBox.flexDirection = juce::FlexBox::Direction::column;
	textEditorBox.justifyContent = juce::FlexBox::JustifyContent::center;
	textEditorBox.items.add(juce::FlexItem(formulaEditor).withFlex(1.0f));
	textEditorBox.items.add(juce::FlexItem(code_console).withMinHeight(40.0f));

	macroSlidersBox.flexDirection = juce::FlexBox::Direction::row;
	macroSlidersBox.flexWrap = juce::FlexBox::Wrap::wrap;
	macroSlidersBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
	for (auto* slider : macroSliders) {
		macroSlidersBox.items.add(juce::FlexItem(*slider).withMinWidth(70.0f).withMinHeight(70.0f));
	}

	mainFlexBox.items.add(juce::FlexItem(textEditorBox).withFlex(1.0f));
	mainFlexBox.items.add(juce::FlexItem(macroSlidersBox).withMinHeight(100.f));

	mainFlexBox.performLayout(getLocalBounds());
}
