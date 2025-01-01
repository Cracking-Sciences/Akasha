
#pragma once
#include <JuceHeader.h>
#include "../console/console.h"
#include "../../kernel/engine/JSEngine.h"
#include "../../utilities/pack.h"
#include "../../utilities/dependencyPath.h"

namespace Akasha {
	inline auto streamToVector(juce::InputStream& stream) {
		using namespace juce;
		std::vector<std::byte> result((size_t)stream.getTotalLength());
		stream.setPosition(0);
		[[maybe_unused]] const auto bytesRead = stream.read(result.data(), result.size());
		jassert(bytesRead == (ssize_t)result.size());
		return result;
	}

	inline const char* getMimeForExtension(const juce::String& extension) {
		using namespace juce;
		static const std::unordered_map<String, const char*> mimeMap =
		{
			{ { "htm"   },  "text/html"                },
			{ { "html"  },  "text/html"                },
			{ { "txt"   },  "text/plain"               },
			{ { "jpg"   },  "image/jpeg"               },
			{ { "jpeg"  },  "image/jpeg"               },
			{ { "svg"   },  "image/svg+xml"            },
			{ { "ico"   },  "image/vnd.microsoft.icon" },
			{ { "json"  },  "application/json"         },
			{ { "png"   },  "image/png"                },
			{ { "css"   },  "text/css"                 },
			{ { "map"   },  "application/json"         },
			{ { "js"    },  "text/javascript"          },
			{ { "woff2" },  "font/woff2"               }
		};
		if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
			return it->second;
		jassertfalse;
		return "";
	}

	class webCodeEditor : public juce::WebBrowserComponent {
	public:
		webCodeEditor(Akasha::JSEngine& engine)
			: juce::WebBrowserComponent(
				juce::WebBrowserComponent::Options{}
				.withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
				.withWinWebView2Options(
					juce::WebBrowserComponent::Options::WinWebView2{}
					.withUserDataFolder(juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)))
				.withResourceProvider([this](const auto& url) {return getResource(url); })
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
			goToURL(getResourceProviderRoot());
		}
		~webCodeEditor() {
			stop();
		}

		void compile() {
			juce::String info;
			giveInfo("Compiling...");
			if (!jsEngine.loadFunction(code.toStdString(), info)) {
				giveInfo(info);
				editAfterCompile = true;
			}
			else {
				giveInfo("Compiled OK :D");
				editAfterCompile = false;
			}
		}

		void setConsole(CodeConsole* console) {
			this->console = console;
		}

		juce::String getText() {
			return code;
		}

		void getTextFromPageAndCompile() {
			juce::String jsCode =
				R"(
    (function() {
		console.log("Backend is reading editor content");
        return editor.getValue();
    })();
 )";
			{
				evaluateJavascript(
					jsCode,
					[&](juce::WebBrowserComponent::EvaluationResult evaluationResult) {
						if (evaluationResult.getResult() == nullptr) {
							giveInfo("Failed to read the code: " + evaluationResult.getError()->message);
						}
						else {
							code = evaluationResult.getResult()->toString();
						}
						// we don't know when will the code be updated due to the async nature of the evaluateJavascript function.
						// so we compile the code in the callback.
						compile();
					}
				);
			}
		}

		void setText(const juce::String& newText) {
			code = newText;
			if (pageLoaded){
				emitEventIfBrowserIsVisible(SetTextEvent, newText);
			}
		}

		void focusGained() {
			hasFocus = true;
			repaint();
		}

		void focusLost() {
			hasFocus = false;
			repaint();
			if (editAfterCompile) {
				giveInfo("");// clear the console
				getTextFromPageAndCompile();
			}
		}

		void paint(juce::Graphics& g) override {
			if (!pageLoaded) {
				g.fillAll(juce::Colours::black);
				g.setColour(juce::Colours::white);
				g.setFont(15.0f);
				g.drawText("Loading...", getLocalBounds(), juce::Justification::centred, true);
			} else {
				juce::WebBrowserComponent::paint(g);
			}
		}

		bool editAfterCompile = false;

		bool isPageLoaded() {
			return pageLoaded;
		}

	private:
		void codeDocumentTextChanged() {
			if (!editAfterCompile) {
				giveInfo("");
			}
			editAfterCompile = true;
		}

		void giveInfo(juce::String info) {
			if (console != nullptr) {
				console->setText(info);
			}
		}

		std::optional<Resource> getResource(const juce::String& url) {
			auto akashaDir = getDependencyPath();
			auto editorFiles = akashaDir.getChildFile("editor_files");
			if (!editorFiles.exists())
				editorFiles.createDirectory();
			auto htmlFile = editorFiles.getChildFile("index.html");
			if (!htmlFile.existsAsFile()) {
				Akasha::unpackBinaryDataToTemp(BinaryData::WebEditorPacked_pack, BinaryData::WebEditorPacked_packSize, editorFiles);
			}
			htmlFile = editorFiles.getChildFile("index.html");
			const auto resourceToTetrive = url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);
			const auto resource = editorFiles.getChildFile(resourceToTetrive).createInputStream();
			if (resource) {
				const auto extension = resourceToTetrive.fromLastOccurrenceOf({ "." }, false, false);
				return Resource{ streamToVector(*resource), getMimeForExtension(extension) };
			}
			return std::nullopt;
		}

		void pageFinishedLoading(const juce::String& url) override {
			pageLoaded = true;
			setText(code);
		}

		CodeConsole* console = nullptr;
		Akasha::JSEngine& jsEngine;
		juce::String code = "nothing yet";
		const juce::Identifier SetTextEvent{ "setText" };
		bool hasFocus = false;
		bool pageLoaded = false;
	};
}