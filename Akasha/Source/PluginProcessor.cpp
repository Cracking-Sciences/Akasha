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
	,
	parameters(*this, nullptr, juce::Identifier("AkashaParameters"), Akasha::createParameterLayout())
{
	for (int i = 0; i < 8; ++i) {
		macros[i] = parameters.getRawParameterValue("macro" + juce::String(i));
	}

	voices.resize(16);
	for (auto i = 0; i < 16; i++) {
		voices[i] = new Akasha::AkashaVoice(jsEngine, i, macros);
		synth.addVoice(voices[i]);
	}
	synth.addSound(new Akasha::AkashaSound());
}

AkashaAudioProcessor::~AkashaAudioProcessor(){
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
	return new AkashaAudioProcessorEditor(*this, parameters);
}

void AkashaAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
	auto state = parameters.copyState();
	std::unique_ptr<juce::XmlElement> xml (state.createXml());

	juce::XmlElement* textData = xml->createNewChildElement("Text");
	if (auto* editor = dynamic_cast<AkashaAudioProcessorEditor*>(getActiveEditor())) {
		textData->setAttribute("code", editor->getCodeString());
		for (int i = 0; i < 8; ++i) {
			textData->setAttribute("macro" + juce::String(i), editor->getMacroText()[i]);
		}
	}

	copyXmlToBinary (*xml, destData);
}

void AkashaAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
	std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
	if (xmlState.get() != nullptr) {
		if (xmlState->hasTagName(parameters.state.getType())) {
			parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
			if (auto* textData = xmlState->getChildByName("Text")) {
				if (auto* editor = dynamic_cast<AkashaAudioProcessorEditor*>(getActiveEditor())) {
					editor->setCodeString(textData->getStringAttribute("code"));
					editor->compile();
					std::array<juce::String, 8> macroText;
					for (int i = 0; i < 8; ++i) {
						macroText[i] = textData->getStringAttribute("macro" + juce::String(i));
					}
					editor->setMacroText(macroText);
				}
			}
		}
	}
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new AkashaAudioProcessor();
}
