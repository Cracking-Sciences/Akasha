#pragma once

#include <JuceHeader.h>
#include "kernel/engine/JSEngine.h"
#include <juce_dsp/juce_dsp.h>
#include "kernel/voice/voice.h"
#include "kernel/engine/defaultJSCode.h"

// sound
namespace Akasha {
	// Adding parameters
	inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
		juce::AudioProcessorValueTreeState::ParameterLayout layout;
		// macros
		for (int i = 0; i < 8; ++i) {
			layout.add(std::make_unique<juce::AudioParameterFloat>
				("macro" + juce::String(i),
					"macro" + juce::String(i),
					juce::Range<float>(0.f, 1.f), 
					0.0f,
					juce::String(), 
					juce::AudioProcessorParameter::genericParameter,
					[](float value, int) { return juce::String(value, 3); },
					[](const juce::String& text) { return text.getFloatValue(); } 
				)
			);
		}
		// editorSize
		layout.add(std::make_unique<juce::AudioParameterInt>(
			"editorWidth", "Editor Width", 1000, std::numeric_limits<int>::max(), 800));
		layout.add(std::make_unique<juce::AudioParameterInt>(
			"editorHeight", "Editor Height", 800, std::numeric_limits<int>::max(), 1000));
		// Oversampling
		layout.add(std::make_unique<juce::AudioParameterInt>("oversampling_factor", "oversampling_factor", 0, 3, 1));
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
	std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
	uint8_t currentOversamplingFactor = 0;
	std::array<std::atomic<float>*, 8> macros;

	void checkOversampler();
};
