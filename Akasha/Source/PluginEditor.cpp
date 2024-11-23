/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "JSEngine.h"

const char* javascriptCode = R"(// javascripts here
function midiNoteToFreq(note){
    if (note < 0 || note > 127) {
        return 0;
    }
    const A4 = 440;
    const A4_note = 69;
    return A4 * Math.pow(2, (note - A4_note)/12);
}

var dcBlockerLastInput = 0;
var dcBlockerLastOutput = 0;
function dcBlocker(input, alpha=0.995){
    const output = input - dcBlockerLastInput + alpha * dcBlockerLastOutput;
    dcBlockerLastInput = input;
    dcBlockerLastOutput = output;
    return output;
}


function main(args){
    var macros = args.macros;
    var tempo = args.tempo;
    var sampleRate = args.sampleRate;
    var time = args.time;
    var beat = args.beat;
    var note = args.note;
    var velocity = args.velocity;
    // calc freq
    var freq = midiNoteToFreq(note);

    var output = time * freq % 1 * 0.5;
    output = dcBlocker(output);
    return output;
}

)";

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
	formulaEditor.setText(javascriptCode);
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
	setSize(600, 400);
	setResizable(true, true);
	setResizeLimits(600, 400, std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

	// processor
	for (auto voice_ptr : audioProcessor.getVoices()) {
		voice_ptr->setConsole(&code_console);
	}

	// First Compile
	formulaEditor.compile();
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
