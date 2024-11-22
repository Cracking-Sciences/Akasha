#include "PluginProcessor.h"
#include "PluginEditor.h"

AkashaAudioProcessor::AkashaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
	voices.resize(16);
	for (auto i = 0; i < 16; i++) {
		voices[i] = new Akasha::AkashaVoice(jsEngine, i, macros);
		synth.addVoice(voices[i]);
	}
	synth.addSound(new Akasha::AkashaSound());
}

AkashaAudioProcessor::~AkashaAudioProcessor() {
}

const juce::String AkashaAudioProcessor::getName() const {
	return JucePlugin_Name;
}

bool AkashaAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool AkashaAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool AkashaAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double AkashaAudioProcessor::getTailLengthSeconds() const {
	return 0.0;
}

int AkashaAudioProcessor::getNumPrograms() {
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int AkashaAudioProcessor::getCurrentProgram() {
	return 0;
}

void AkashaAudioProcessor::setCurrentProgram(int index) {
}

const juce::String AkashaAudioProcessor::getProgramName(int index) {
	return {};
}

void AkashaAudioProcessor::changeProgramName(int index, const juce::String& newName) {
}

void AkashaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
	synth.setCurrentPlaybackSampleRate(sampleRate);

}

void AkashaAudioProcessor::releaseResources() {
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AkashaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void AkashaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	auto playHead = getPlayHead();
	if (playHead != nullptr) {
		juce::AudioPlayHead::CurrentPositionInfo position;
		if (playHead->getCurrentPosition(position)) {
			beat = position.ppqPosition;
			tempo = position.bpm;
		}
	}
	for (auto voice_ptr : voices) {
		voice_ptr->setGlobalParams(tempo, beat, getSampleRate());
	}
	synth.setCurrentPlaybackSampleRate(getSampleRate());
	synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

}

bool AkashaAudioProcessor::hasEditor() const {
	return true;
}

juce::AudioProcessorEditor* AkashaAudioProcessor::createEditor() {
	return new AkashaAudioProcessorEditor(*this);
}

void AkashaAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void AkashaAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new AkashaAudioProcessor();
}
