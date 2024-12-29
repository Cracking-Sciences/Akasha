
#pragma once
#include <JuceHeader.h>
#include "../../kernel/engine/JSEngine.h"
namespace Akasha {
	class AkashaSound : public juce::SynthesiserSound {
	public:
		AkashaSound() {}
		bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
		bool appliesToChannel(int /*midiChannel*/) override { return true; }
	};

	// voice
	class AkashaVoice : public juce::SynthesiserVoice {
	public:
		AkashaVoice(JSEngine& engine, int voiceId, std::array<std::atomic<float>*, 8> macros) :
			jsEngine(engine),
			voiceId(voiceId)
		{
			recv_macros = macros;
			for (size_t i = 0; i < 8; i++) {
				mainWrapperParams.macros[i] = new std::atomic<float>(0);
			}
		}

		~AkashaVoice() {
			for (size_t i = 0; i < 8; i++) {
				delete mainWrapperParams.macros[i];
			}
		}

		bool canPlaySound(juce::SynthesiserSound* sound) override {
			return dynamic_cast<AkashaSound*>(sound) != nullptr;
		}

		void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override {
			mainWrapperParams.voiceId = voiceId;
			mainWrapperParams.note = midiNoteNumber;
			mainWrapperParams.velocity = velocity;
			mainWrapperParams.justPressed = true;
			held = true;
		}

		void stopNote(float velocity, bool allowTailOff) override {
			mainWrapperParams.justPressed = false;
			mainWrapperParams.velocity = velocity;
			held = false;
			clearCurrentNote();
		}

		void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
			if (!jsEngine.isFunctionReady()) {
				mainWrapperParams.justPressed = true; // so it can continue when fixed.
				return;
			}
			if (jsEngine.isFunctionJustReadyForVoice(voiceId)) {
				mainWrapperParams.justPressed = true; // so it can continue when fixed.
			}
			if (!held) {
				return;
			}
			mainWrapperParams.numSamples = numSamples;
			mainWrapperParams.numChannels = outputBuffer.getNumChannels();
			mainWrapperParams.sampleRate = getSampleRate();

			for (size_t macro_index = 0; macro_index < 8; macro_index++) {
				mainWrapperParams.macros[macro_index]->store(*recv_macros[macro_index]);
			}

			juce::String info;
			if (!jsEngine.callMainWrapperFunction(mainWrapperParams, outputBuffer, startSample, mainWrapperParams.numSamples, info, voiceId)) {
				giveInfo(info);
				mainWrapperParams.justPressed = true; // so it can continue when fixed.
				return;
			}
			else {
				mainWrapperParams.justPressed = false;
			}
		}

		void pitchWheelMoved(int) override {};

		void controllerMoved(int, int) override {};

		void setGlobalParams(double tempo, double beat) {
			// called by processor
			mainWrapperParams.tempo = tempo;
			mainWrapperParams.beat = beat;
		}

		void setConsole(juce::TextEditor* code_console) {
			//called by editor
			this->code_console = code_console;
		}

		void giveInfo(juce::String info) {
			if (code_console != nullptr) {
				code_console->setText(info);
			}
		}

	private:
		JSMainWrapperParams mainWrapperParams;
		JSEngine& jsEngine;
		juce::TextEditor* code_console = nullptr;
		int voiceId;
		bool held = false;

		std::array<std::atomic<float>*, 8> recv_macros;
	};


}
