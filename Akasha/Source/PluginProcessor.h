#pragma once

#include <JuceHeader.h>
#include "JSEngine.h"

// sound
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
		AkashaVoice(JSEngine& engine, int voiceId, std::array<double, 8>& macrosRef) :
			jsEngine(engine),
			voiceId(voiceId),
			params(macrosRef) {
		}
		bool canPlaySound(juce::SynthesiserSound* sound) override {
			return dynamic_cast<AkashaSound*>(sound) != nullptr;
		}

		void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override {
			params.note = midiNoteNumber;
			params.velocity = velocity;
			params.time = 0.0;
			held = true;
		}

		void stopNote(float velocity, bool allowTailOff) override {
			clearCurrentNote();
			params.time = 0.0;
			held = false;
		}

		void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
			if (!jsEngine.isFunctionReady()) {
				return;
			}
			if (!held) {
				return;
			}
			params.sampleRate = getSampleRate();
			std::vector<double> result_vector;
			result_vector.resize(outputBuffer.getNumChannels());
			juce::String info;
			for (size_t i = 0; i < numSamples; i++) {
				if (!jsEngine.callFunction(params, result_vector, info, voiceId)) {
					giveInfo(info);
					return;
				}
				for (size_t j = 0; j < outputBuffer.getNumChannels(); j++) {
					outputBuffer.addSample(j, startSample + i, result_vector[j]);
				}
				params.time += 1.0 / params.sampleRate;
			}
		}

		void pitchWheelMoved(int) override {};

		void controllerMoved(int, int) override {};

		void setMacros(std::array<double, 8>& macros) {
			// called by editor
			params.macros = macros;
		}

		void setGlobalParams(double tempo, double beat, double sampleRate) {
			// called by processor
			params.tempo = tempo;
			params.beat = beat;
			params.sampleRate = sampleRate;
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
		JSFuncParams params;
		JSEngine& jsEngine;
		juce::TextEditor* code_console = nullptr;
		int voiceId;
		bool held = false;
	};

}


class AkashaAudioProcessor : public juce::AudioProcessor {
public:

	AkashaAudioProcessor();
	~AkashaAudioProcessor() override;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	Akasha::JSEngine& getJSEngine() { return jsEngine; }
	void setMacros(std::array<double, 8> macros) {
		this->macros = macros;
	}

	std::vector<Akasha::AkashaVoice*>& getVoices() {
		return voices;
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AkashaAudioProcessor)
		Akasha::JSEngine jsEngine;
	double tempo = 0.;
	double beat = 0.;
	juce::Synthesiser synth;
	std::vector<Akasha::AkashaVoice*> voices;
	std::array<double, 8> macros;
};
