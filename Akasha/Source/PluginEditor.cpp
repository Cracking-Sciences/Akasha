/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

AkashaAudioProcessorEditor::AkashaAudioProcessorEditor(AkashaAudioProcessor& p, juce::AudioProcessorValueTreeState& vts):
	AudioProcessorEditor(&p),
	valueTreeState (vts),
	audioProcessor(p),
	// codeDocument(),
	// codeTokeniser(),
	// formulaEditorPointer(std::make_unique<Akasha::builtinFormulaEditor>(codeDocument, &codeTokeniser, audioProcessor.getJSEngine())),
	formulaEditorPointer(std::make_unique<Akasha::webCodeEditor>(audioProcessor.getJSEngine())),
	macroSliderGroupPointer(std::make_unique<Akasha::Macros>(vts)),
	codeConsolePointer(std::make_unique<Akasha::CodeConsole>()),
	oversamplingBoxPointer(std::make_unique<Akasha::OversamplingBox>("oversampling 2^", vts, "oversampling_factor"))
{
	// juce::LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);
	setLookAndFeel(&customLookAndFeel);
	// macros
	addAndMakeVisible(macroSliderGroupPointer.get());
	// console.
	addAndMakeVisible(codeConsolePointer.get());
	// code editor.
	formulaEditorPointer->setConsole(codeConsolePointer.get());
	addAndMakeVisible(formulaEditorPointer.get());
	formulaEditorPointer->editAfterCompile = false;
	// oversampling
	addAndMakeVisible(oversamplingBoxPointer.get());

	// main editor.
	int savedWidth = valueTreeState.getParameterAsValue("editorWidth").getValue();
	int savedHeight = valueTreeState.getParameterAsValue("editorHeight").getValue();
	setSize(savedWidth, savedHeight);
	setResizable(true, true);
	setResizeLimits(1000, 800, std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	// processor
	for (auto voice_ptr : audioProcessor.getVoices()) {
		voice_ptr->setConsole(codeConsolePointer.get());
	}
	// read other states
	setCodeString(audioProcessor.savedCode);
	setMacroText(audioProcessor.savedMacroText);

	// auto* topLevel = juce::TopLevelWindow::getTopLevelWindow(0);
	// if (topLevel != nullptr) {
	// 	topLevel->setUsingNativeTitleBar(true);
	// }
}

AkashaAudioProcessorEditor::~AkashaAudioProcessorEditor() {
	// report current texts before closing.
	audioProcessor.savedCode = getCodeString();
	audioProcessor.savedMacroText = getMacroText();
	setLookAndFeel(nullptr);
}

void AkashaAudioProcessorEditor::paint(juce::Graphics& g) {
	g.fillAll(juce::Colours::darkgrey);
}

/*
void AkashaAudioProcessorEditor::mouseDown(const juce::MouseEvent& event) {
	if (!formulaEditorPointer->getBounds().contains(event.getPosition())) {
		formulaEditorPointer->unfocusAllComponents();
	}
	AudioProcessorEditor::mouseDown(event);
}
*/

void AkashaAudioProcessorEditor::resized() {
	juce::FlexBox mainFlexBox;
	juce::FlexBox textEditorBox;
	juce::FlexBox buttomWidgetsBox;

	mainFlexBox.flexDirection = juce::FlexBox::Direction::column;

	{
		textEditorBox.flexDirection = juce::FlexBox::Direction::column;
		textEditorBox.justifyContent = juce::FlexBox::JustifyContent::center;
		textEditorBox.items.add(juce::FlexItem(*formulaEditorPointer).withFlex(1.0f)
			.withMargin(juce::FlexItem::Margin(0.0f)));
		textEditorBox.items.add(juce::FlexItem(*codeConsolePointer).withMinHeight(40.0f)
			.withMargin(juce::FlexItem::Margin(0.0f)));
	}

	{	buttomWidgetsBox.flexDirection = juce::FlexBox::Direction::row;
		buttomWidgetsBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
		buttomWidgetsBox.items.add(juce::FlexItem(*macroSliderGroupPointer).withMinWidth(700.0f));
		buttomWidgetsBox.items.add(juce::FlexItem(*oversamplingBoxPointer).withMinWidth(180.0f));
	}


	mainFlexBox.items.add(juce::FlexItem(textEditorBox).withFlex(1.0f));
	mainFlexBox.items.add(juce::FlexItem(buttomWidgetsBox).withMinHeight(90.0f).withMinWidth(getWidth())); 

	mainFlexBox.performLayout(getLocalBounds().reduced(3.0f));

	valueTreeState.getParameterAsValue("editorWidth").setValue(getWidth());
	valueTreeState.getParameterAsValue("editorHeight").setValue(getHeight());
}

juce::String AkashaAudioProcessorEditor::getCodeString() const {
	return formulaEditorPointer->getText();
}

void AkashaAudioProcessorEditor::setCodeString(const juce::String& newText) {
	formulaEditorPointer->setText(newText);
}

void AkashaAudioProcessorEditor::setMacroText(const std::array<juce::String, 8>& newText) {
	for (int i = 0; i < 8; ++i) {
		macroSliderGroupPointer->setMacroText(i, newText[i]);
	}
}

const std::array<juce::String, 8> AkashaAudioProcessorEditor::getMacroText() {
	std::array<juce::String, 8> result;
	for (int i = 0; i < 8; ++i) {
		result[i] = macroSliderGroupPointer->getMacroText(i);
	}
	return result;
}

void AkashaAudioProcessorEditor::compile() {
	formulaEditorPointer->compile();
}