/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "JSEngine.h"
#include "ui/MyLookAndFeel.h"

#include "ui/console/console.h"
#include "ui/macros/macros.h"

#include "assets/JsTokeniser/JavascriptCodeTokeniser.h"

//==============================================================================
/**
*/

namespace Akasha {
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
		formulaEditor.compile();
	}


private:
	AkashaAudioProcessor& audioProcessor;
	juce::AudioProcessorValueTreeState& valueTreeState;

	// code editor.
	juce::CodeDocument codeDocument;
	JavascriptTokeniser codeTokeniser;
	std::unique_ptr<Akasha::FormulaEditor> formulaEditorPointer;
	Akasha::FormulaEditor& formulaEditor;
	// code console.
	Akasha::CodeConsole code_console;
	// macro sliders.
	std::unique_ptr<Akasha::Macros> macroSliderGroupPointer;
	// custom look and feel.
	Akasha::CustomLookAndFeel customLookAndFeel;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AkashaAudioProcessorEditor)
};
