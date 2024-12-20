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
	// default
	savedCode = defaultJavascriptCode;
	for (int i = 0; i < 8; ++i) {
		savedMacroText[i] = "m" + juce::String(i);
	}
	juce::String dummy;
	jsEngine.loadFunction(savedCode.toStdString(), dummy);
}

AkashaAudioProcessor::~AkashaAudioProcessor(){
	synth.clearVoices();
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
	if (auto* editor = dynamic_cast<AkashaAudioProcessorEditor*>(getActiveEditor())) {
		savedCode = editor->getCodeString();
		savedMacroText = editor->getMacroText();
	}
	auto state = parameters.copyState();
	std::unique_ptr<juce::XmlElement> xml (state.createXml());
	if (xml != nullptr) {
		while (auto* existingText = xml->getChildByName("Text")) {
			xml->removeChildElement(existingText, true);
		}
		juce::XmlElement* textData = xml->createNewChildElement("Text");
		if (textData != nullptr) {
			textData->setAttribute("code", savedCode);
			for (int i = 0; i < 8; ++i) {
				textData->setAttribute("macro" + juce::String(i), savedMacroText[i]);
			}
		}
		copyXmlToBinary(*xml, destData);
	}
	DBG("Generated XML: " + xml->toString()); // debug
}

void AkashaAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
	if (data == nullptr || sizeInBytes == 0) {
		savedCode = defaultJavascriptCode;
		for (int i = 0; i < 8; ++i) {
			savedMacroText[i] = "m" + juce::String(i);
		}
	}
	else{
		std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
		DBG("Loaded XML: " + xmlState->toString()); // debug
		if (xmlState.get() != nullptr) {
			if (xmlState->hasTagName(parameters.state.getType())) {
				parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
				if (auto* textData = xmlState->getChildByName("Text")) {
					savedCode = textData->getStringAttribute("code", defaultJavascriptCode);
					for (int i = 0; i < 8; ++i) {
						savedMacroText[i] = textData->getStringAttribute("macro" + juce::String(i), "m" + juce::String(i));
					}
				}
			}
		}
	}
	if (auto* editor = dynamic_cast<AkashaAudioProcessorEditor*>(getActiveEditor())) {
		editor->setCodeString(savedCode);
		editor->setMacroText(savedMacroText);
		editor->compile();
	}
	else {
		juce::String dummy;
		jsEngine.loadFunction(savedCode.toStdString(), dummy);
	}
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new AkashaAudioProcessor();
}


const char* const defaultJavascriptCode = R"(// javascripts here
function midiNoteToFreq(note){
    if (note < 0 || note > 127) {
        return 0;
    }
    const A4 = 440;
    const A4_note = 69;
    return A4 * Math.pow(2, (note - A4_note)/12);
}

var dcBlockerLastInput = 0;
var dcBlockerLastOutput = 0;
function dcBlocker(input, alpha=0.995){
    const output = input - dcBlockerLastInput + alpha * dcBlockerLastOutput;
    dcBlockerLastInput = input;
    dcBlockerLastOutput = output;
    return output;
}

var phase = 0.0

function main(args){
    var [m0,m1,m2,m3,m4,m5,m6,m7, tempo, beat, sampleRate, bufferLen, bufferPos, 
        time, note, velocity, justPressed, justReleased
        ] = args;
    // calc freq
    var freq = midiNoteToFreq(note);
    phase += freq / sampleRate;
    phase %= 1.0;

    var output = (phase % 1 - 0.5) * 0.5;
    output = dcBlocker(output);
    return output;
}
)";