/*
  ==============================================================================

    builtin_code_editor.h
    Created: 22 Dec 2024 11:39:24pm
    Author:  ric

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "JsTokeniser/JavascriptCodeTokeniser.h"
#include "../console/console.h"
#include "../../kernel/engine/JSEngine.h"

namespace Akasha {
	class builtinFormulaEditor : public juce::CodeEditorComponent,
		private juce::CodeDocument::Listener {
	public:
		builtinFormulaEditor(juce::CodeDocument& document, juce::CodeTokeniser* codeTokeniser, Akasha::JSEngine& engine) :
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
		void setConsole(CodeConsole* console) {
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

		CodeConsole* console = nullptr; // debug purpose.
		Akasha::JSEngine& jsEngine;
		bool hasFocus = false; // keyboard focus
	};
}