
#pragma once
#include <JuceHeader.h>
#include "../console/console.h"
#include "../../kernel/engine/JSEngine.h"
#include "../../utilities/pack.h"

namespace Akasha {
    class webCodeEditor : public juce::WebBrowserComponent {
    public:
        webCodeEditor(Akasha::JSEngine& engine)
            : juce::WebBrowserComponent(
                juce::WebBrowserComponent::Options{}
                .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                .withWinWebView2Options(
                    juce::WebBrowserComponent::Options::WinWebView2{}
                    .withUserDataFolder(juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)))
                .withNativeIntegrationEnabled()
                .withNativeFunction("edited", [this](auto& args, auto complete) {
                    codeDocumentTextChanged();
                    complete("Editor state updated successfully.");
                    })
                .withNativeFunction("focusGained", [this](auto& args, auto complete) {
                    focusGained();
                    complete("Focus gained handled.");
                    })
                .withNativeFunction("focusLost", [this](auto& args, auto complete) {
                    focusLost();
                    complete("Focus lost handled.");
                    })
            ),
            jsEngine(engine) {

            setWantsKeyboardFocus(true);

            // Define the temporary directory
            auto baseDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
            auto crackingSciencesDir = baseDir.getChildFile("CrackingSciences");
            if (!crackingSciencesDir.exists())
                crackingSciencesDir.createDirectory();
            auto akashaDir = crackingSciencesDir.getChildFile("Akasha");
            if (!akashaDir.exists())
                akashaDir.createDirectory();
            auto csAkashaFilesDir = akashaDir.getChildFile("cs_akasha_files");
            if (!csAkashaFilesDir.exists())
                csAkashaFilesDir.createDirectory();
            auto& tempDir = csAkashaFilesDir;
            // DBG("webpage directory: " + tempDir.getFullPathName());
            auto htmlFile = tempDir.getChildFile("index.html");
            if (!htmlFile.existsAsFile()) {
                Akasha::unpackBinaryDataToTemp(BinaryData::WebEditorPacked_pack, BinaryData::WebEditorPacked_packSize, tempDir);
            }
            htmlFile = tempDir.getChildFile("index.html");
            if (htmlFile.existsAsFile())
            {
                goToURL("file:///" + htmlFile.getFullPathName());
            }
            else
            {
                // Fallback to a URL if the HTML file is missing
                goToURL("https://example.com");
            }
		}
		~webCodeEditor() {
			stop();
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

        void setConsole(CodeConsole* console) {
            this->console = console;
        }

        juce::String getText() {
            juce::String jsCode = 
R"(
    (function() {
        return document.getElementById('editor') ? document.getElementById('editor').value : "";
    })();
 )";
            juce::String result;
            evaluateJavascript(jsCode, [&](juce::WebBrowserComponent::EvaluationResult evaluationResult){
				if (evaluationResult.getResult()==nullptr) {
                    giveInfo("Failed to get code.");
					return;
				}
                result = *evaluationResult.getResult();
                });

            return result;
        }

		void setText(const juce::String& newText) {
            juce::String jsCode = 
R"(
    (function() {
        var editor = document.getElementById('editor');
        if (!editor) {
            throw new Error("Editor not found");
        }
        editor.value = 
)" + 
newText.quoted() + 
R"(
        ;
    })();
)";
            evaluateJavascript(jsCode, [&](juce::WebBrowserComponent::EvaluationResult evaluationResult) {
                if (evaluationResult.getResult()==nullptr) {
                    giveInfo("Failed to set code");
                    return;
                }
                });
        }

        void focusGained()  {
			giveInfo("Focus gained.");
			hasFocus = true;
			repaint();
		}

		void focusLost() {
			giveInfo("Focus lost.");
			hasFocus = false;
			repaint();
            if (editAfterCompile) {
				compile();
            }
		}

		void paint(juce::Graphics& g) override {
            juce::WebBrowserComponent::paint(g);
            if (!hasFocus) {
                juce::Colour overlayColour = juce::Colours::grey.withAlpha(0.2f);
                auto bounds = getLocalBounds();
                g.setColour(overlayColour);
                g.fillRect(bounds);
            }
		}

        bool editAfterCompile = false;

    private:
		void codeDocumentTextChanged() {
			editAfterCompile = true;
		}

        void giveInfo(juce::String info) {
            if (console != nullptr) {
                console->setText(info);
            }
        }

        CodeConsole* console = nullptr; 
        Akasha::JSEngine& jsEngine;
        bool hasFocus = false;
    };
}