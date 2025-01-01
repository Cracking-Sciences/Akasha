
#pragma once
#include <JuceHeader.h>
#include "../../ui/adsr/adsr.h"
#include "../../kernel/engine/JSEngine.h"
namespace Akasha {

	class AkashaSynthesiser : public juce::Synthesiser {
	public:
		AkashaSynthesiser() = default;
		/*
		void noteOn(int midiChannel, int midiNoteNumber, float velocity) override {
			const juce::ScopedLock sl(lock);
		
			for (auto* sound : sounds) {
				if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(midiChannel)) {
					// Remove the following logic to allow multiple instances of the same note:
					
					for (auto* voice : voices)
					if (voice->getCurrentlyPlayingNote() == midiNoteNumber && voice->isPlayingChannel(midiChannel))
					stopVoice(voice, 1.0f, true);
					
		
					startVoice(findFreeVoice(sound, midiChannel, midiNoteNumber, isNoteStealingEnabled()),
						sound, midiChannel, midiNoteNumber, velocity);
				}
			}
		}
		*/
	};

	class AkashaSound : public juce::SynthesiserSound {
	public:
		AkashaSound() {}
		bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
		bool appliesToChannel(int /*midiChannel*/) override { return true; }
	};

	// voice
	class AkashaVoice : public juce::SynthesiserVoice {
	public:
		AkashaVoice(JSEngine& engine, ADSRKernel& adsrKernel, int voiceId, std::array<std::atomic<float>*, 8> macros) :
			jsEngine(engine),
			adsrKernel(adsrKernel),
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

			inNoteLife = true;
			noteReleased = false;
			time = 0.0;
		}

		void stopNote(float velocity, bool allowTailOff) override {
			if (!noteReleased) {
				// when the second same note triggered off, 
				// the first note will also be triggered off (the second time!)
				// we don't want to set timeAtRelease twice.
				mainWrapperParams.justPressed = false;
				mainWrapperParams.velocity = velocity;
				noteReleased = true;
				timeAtRelease = time;
			}
			if (!allowTailOff) {
				inNoteLife = false;
				clearCurrentNote();
			}
		}

		void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
			double sampleRate = getSampleRate();
			if (sampleRate <= 0) {
				return;
			}
			if (!jsEngine.isFunctionReady() || !inNoteLife) {
				time += numSamples / sampleRate;
				return;
			}
			if (jsEngine.isFunctionJustReadyForVoice(voiceId)) {
				mainWrapperParams.justPressed = true; // so it can continue when fixed.
			}
			mainWrapperParams.numSamples = numSamples;
			mainWrapperParams.numChannels = outputBuffer.getNumChannels();
			mainWrapperParams.time = time;

			for (size_t macro_index = 0; macro_index < 8; macro_index++) {
				mainWrapperParams.macros[macro_index]->store(*recv_macros[macro_index]);
			}

			// process the whole buffer
			juce::String info;
			channelDataList.resize(outputBuffer.getNumChannels());
			if (!jsEngine.callMainWrapperFunction(mainWrapperParams, 0, mainWrapperParams.numSamples, info, voiceId, channelDataList)) {
				giveInfo(info);
				time += numSamples / sampleRate;
				return;
			}
			else {
				mainWrapperParams.justPressed = false;
			}

			// ADSR
			bool stop = false;
			float v = 1.0f;
			for (int sample = 0; sample < numSamples; ++sample) {
				float timeNow = time + sample / sampleRate;
				adsrKernel.getValue(timeNow, noteReleased, timeAtRelease, v, stop);
				for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
					channelDataList[channel][sample] *= v;
				}
			}
			for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
				outputBuffer.addFrom(channel, startSample, channelDataList[channel], numSamples);
			}
			if (stop) {
				clearCurrentNote();
				inNoteLife = false;
			}
			// Increment time by the number of samples
			time += numSamples / sampleRate;
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
		Akasha::ADSRKernel& adsrKernel;
		std::vector<float*> channelDataList; // the buffer result from the jsEngine
		juce::TextEditor* code_console = nullptr;
		int voiceId;
		bool inNoteLife = false;
		bool noteReleased = false;
		float time = 0.0;
		float timeAtRelease = 0.0;

		std::array<std::atomic<float>*, 8> recv_macros;
	};


}
