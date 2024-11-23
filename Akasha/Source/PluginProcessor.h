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
		AkashaVoice(JSEngine& engine, int voiceId, std::array<std::atomic<float>*, 8> macros) :
			jsEngine(engine),
			voiceId(voiceId)
		{
			recv_macros = macros;
			// for macro smoothing
			for (size_t i = 0; i < 8; i++) {
				prev_macros[i] = *macros[i];
				current_macros[i] = *macros[i];
				params.macros[i] = new std::atomic<float>(current_macros[i]);
			}
		}

		~AkashaVoice() {
			for (size_t i = 0; i < 8; i++) {
				delete params.macros[i];
			}
		}

		bool canPlaySound(juce::SynthesiserSound* sound) override {
			return dynamic_cast<AkashaSound*>(sound) != nullptr;
		}

		void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override {
			params.note = midiNoteNumber;
			params.velocity = velocity;
			params.time = 0.0;
			params.justPressed = true;
			held = true;
		}

		void stopNote(float velocity, bool allowTailOff) override {
			clearCurrentNote();
			params.time = 0.0;
			params.justPressed = false;
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
			params.bufferLen = numSamples;
			for (size_t macro_index = 0; macro_index < 8; macro_index++) {
				current_macros[macro_index] = *recv_macros[macro_index];
			}
			std::vector<double> result_vector;
			result_vector.resize(outputBuffer.getNumChannels());
			juce::String info;
			for (size_t i = 0; i < numSamples; i++) {
				params.bufferPos = i;
				for (size_t macro_index = 0; macro_index < 8; macro_index++) {
					params.macros[macro_index]->store(
						(float(i + 1) / numSamples) * current_macros[macro_index] +
						(1.0f - float(i + 1) / numSamples) * prev_macros[macro_index]
					);
				}

				if (!jsEngine.callFunction(params, result_vector, info, voiceId)) {
					giveInfo(info);
					return;
				}
				for (size_t j = 0; j < outputBuffer.getNumChannels(); j++) {
					outputBuffer.addSample(j, startSample + i, result_vector[j]);
				}
				params.time += 1.0 / params.sampleRate;
				params.justPressed = false;
			}

			for (size_t macro_index = 0; macro_index < 8; macro_index++) {
				prev_macros[macro_index] = current_macros[macro_index];
			}
		}

		void pitchWheelMoved(int) override {};

		void controllerMoved(int, int) override {};

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

		std::array<std::atomic<float>*, 8> recv_macros;
		std::array<float, 8> prev_macros;
		std::array<float, 8> current_macros;
	};

	// Adding parameters
	inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
		juce::AudioProcessorValueTreeState::ParameterLayout layout;
		// macros
		for (int i = 0; i < 8; ++i) {
			layout.add(std::make_unique<juce::AudioParameterFloat>("macro" + juce::String(i), "macro" + juce::String(i), 0.0f, 1.0f, 0.0f));
		}
		return layout;
	}
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
	juce::AudioProcessorValueTreeState parameters;
	std::array<std::atomic<float>*, 8> macros;
};
