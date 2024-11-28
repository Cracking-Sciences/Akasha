#pragma once

#include <JuceHeader.h>
#include "JSEngine.h"

extern const char* const defaultJavascriptCode;

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
				mainWrapperParams.macros[i] = new std::atomic<float>(current_macros[i]);
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
			mainWrapperParams.note = midiNoteNumber;
			mainWrapperParams.velocity = velocity;
			mainWrapperParams.justPressed = true;
			held = true;
		}

		void stopNote(float velocity, bool allowTailOff) override {
			mainWrapperParams.justPressed = false;
			mainWrapperParams.justReleased = true;
			mainWrapperParams.velocity = velocity;
			held = false;
			clearCurrentNote();
		}

		void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
			if (!jsEngine.isFunctionReady()) {
				return;
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
			if (!jsEngine.callMainWrapperFunction(mainWrapperParams, outputBuffer, startSample, numSamples, info, voiceId)) {
				giveInfo(info);
				return;
			}
			mainWrapperParams.justPressed = false;
			mainWrapperParams.justReleased = false;
		}

		void pitchWheelMoved(int) override {};

		void controllerMoved(int, int) override {};

		void setGlobalParams(double tempo, double beat, double sampleRate) {
			// called by processor
			mainWrapperParams.tempo = tempo;
			mainWrapperParams.beat = beat;
			mainWrapperParams.sampleRate = sampleRate;
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
		// editorSize
		layout.add(std::make_unique<juce::AudioParameterInt>(
			"editorWidth", "Editor Width", 600, std::numeric_limits<int>::max(), 800));
		layout.add(std::make_unique<juce::AudioParameterInt>(
			"editorHeight", "Editor Height", 600, std::numeric_limits<int>::max(), 1000));
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

	juce::String savedCode;
	std::array<juce::String, 8> savedMacroText;

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
