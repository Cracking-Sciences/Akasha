/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "JSEngine.h"
#include "MyLookAndFeel.h"
#include "assets/JsTokeniser/JavascriptCodeTokeniser.h"

//==============================================================================
/**
*/

namespace Akasha {
	class SliderWithLabel : public juce::Component {
	public:
		SliderWithLabel(const juce::String& labelText) {
			slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
			slider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, textWidth, 20);
			slider.setRange(0.0, 1.0);
			// slider.setNumDecimalPlacesToDisplay(4);
			addAndMakeVisible(slider);

			label.setText(labelText, juce::dontSendNotification);
			label.setJustificationType(juce::Justification::centred);
			label.setMinimumHorizontalScale(0.3f); 
			label.setEditable(true);
			addAndMakeVisible(label);
		}

		juce::Slider& getSlider() { return slider; }
		juce::Label& getLabel() { return label; }

		void resized() override {
			juce::FlexBox flexBox;
			flexBox.flexDirection = juce::FlexBox::Direction::column;
			flexBox.items.add(juce::FlexItem(label).withMinHeight(10.0f).withFlex(1.0f));
			flexBox.items.add(juce::FlexItem(slider).withFlex(5.0f));
			flexBox.performLayout(getLocalBounds());
		}

		juce::String getLabelText() {
			return label.getText();
		}

		void setLabelText(const juce::String& newText) {
			label.setText(newText, juce::dontSendNotification);
		}

		float getTextWidth() {
			return textWidth;
		}

	private:
		juce::Slider slider;
		juce::Label label;
		float textWidth = 80.0f;
	};

	class FormulaEditor : public juce::CodeEditorComponent,
		private juce::CodeDocument::Listener {
	public:
		FormulaEditor(juce::CodeDocument& document, juce::CodeTokeniser* codeTokeniser, Akasha::JSEngine& engine) :
			juce::CodeEditorComponent(document, codeTokeniser), jsEngine(engine) {
			setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::plain));
			setTabSize(4, true);

			document.addListener(this);
			setScrollbarThickness(8);

			editAfterCompile = false;
		}

		void compile() {
			juce::String info;
			giveInfo("Compiling...");
			if (!jsEngine.loadFunction(getText().toStdString(), info)) {
				giveInfo(info);
			}
			else {
				giveInfo("Compiled OK :D");
			}
			editAfterCompile = false;
		}

		bool keyPressed(const juce::KeyPress& key) override {
			if (key == juce::KeyPress(juce::KeyPress::returnKey, juce::ModifierKeys::shiftModifier, NULL)) {
				compile();
				return true;
			}
			return juce::CodeEditorComponent::keyPressed(key);
		}
		void setConsole(juce::TextEditor* console) {
			this->console = console;
		}

		juce::String getText() const {
			return getDocument().getAllContent();
		}

		void setText(const juce::String& newText) {
			loadContent(newText);
		}

		void focusGained(FocusChangeType cause) override {
			hasFocus = true;
			repaint();
		}
		void focusLost(FocusChangeType cause) override {
			hasFocus = false;
			repaint();
			// auto recompile : )
			if (editAfterCompile) {
				compile();
			}
		}

		void paint(juce::Graphics& g) override {
			juce::CodeEditorComponent::paint(g);
			if (!hasFocus) {
				juce::Colour overlayColour = juce::Colours::grey.withAlpha(0.2f);
				auto bounds = getLocalBounds();
				g.setColour(overlayColour);
				g.fillRect(bounds);
			}
		}

		bool editAfterCompile = false;

	private:
		void codeDocumentTextInserted(const juce::String&, int) override {
			editAfterCompile = true;
		}
		void codeDocumentTextDeleted(int, int) override {
			editAfterCompile = true;
		}

		void giveInfo(juce::String info) {
			if (console != nullptr) {
				console->setText(info);
			}
		}

		juce::TextEditor* console = nullptr; // debug purpose.
		Akasha::JSEngine& jsEngine;
		bool hasFocus = false; // keyboard focus
	};
}

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


class AkashaAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
	AkashaAudioProcessorEditor(AkashaAudioProcessor& p, juce::AudioProcessorValueTreeState& vts);
	~AkashaAudioProcessorEditor() override;

	void paint(juce::Graphics&) override;
	void resized() override;
	void mouseDown(const juce::MouseEvent& event) override;

	juce::String getCodeString() const {
		return formulaEditor.getText();
	}

	void setCodeString(const juce::String& newText) {
		formulaEditor.setText(newText);
	}

	void setMacroText(const std::array<juce::String, 8>& newText) {
		for (int i = 0; i < 8; ++i) {
			macroSliders[i]->setLabelText(newText[i]);
		}
	}

	const std::array<juce::String, 8> getMacroText() {
		std::array<juce::String, 8> result;
		for (int i = 0; i < 8; ++i) {
			result[i] = macroSliders[i]->getLabelText();
		}
		return result;
	}

	void compile() {
		formulaEditor.compile();
	}

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

private:
	AkashaAudioProcessor& audioProcessor;
	juce::AudioProcessorValueTreeState& valueTreeState;

	// macro sliders.
	juce::OwnedArray<Akasha::SliderWithLabel> macroSliders;
	juce::OwnedArray<SliderAttachment> macroSliderAttachments;
	// code editor.
	juce::CodeDocument codeDocument;
	JavascriptTokeniser codeTokeniser;
	std::unique_ptr<Akasha::FormulaEditor> formulaEditorPointer;
	Akasha::FormulaEditor& formulaEditor;
	// code console.
	CodeConsole code_console;
	// custom look and feel.
	Akasha::CustomLookAndFeel customLookAndFeel;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AkashaAudioProcessorEditor)
};
