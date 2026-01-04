#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(2, 2, 
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Core Controls
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 1.0f, 30.0f, 10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("out", "Output", -24.0f, 24.0f, 0.0f));

    // 3-Band EQ with sweepable Middle Frequency
    layout.add(std::make_unique<juce::AudioParameterFloat>("bass", "Bass", -12.0f, 12.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("middle", "Middle", -12.0f, 12.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("treble", "Treble", -12.0f, 12.0f, 0.0f));
    
    // Frequency range from 200Hz to 5kHz with a skew factor for log-like behavior
    auto freqRange = juce::NormalisableRange<float>(200.0f, 5000.0f, 1.0f, 0.5f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("frequency", "Mid Freq", freqRange, 750.0f));

    return layout;
}

void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    oversampler->initProcessing (static_cast<size_t>(samplesPerBlock));
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate * static_cast<double>(oversampler->getOversamplingFactor());
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock) * static_cast<juce::uint32>(oversampler->getOversamplingFactor());
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    postEQ.prepare(spec);
    updateFilters();
}

void AudioPluginAudioProcessor::updateFilters()
{
    const double osRate = getSampleRate() * static_cast<double>(oversampler->getOversamplingFactor());
    
    auto getVal = [this](juce::String id) { return apvts.getRawParameterValue(id)->load(); };
    auto getG = [&](juce::String id) { return juce::Decibels::decibelsToGain(getVal(id)); };

    *postEQ.template get<BassIndex>().coefficients = *FilterCoefs::makeLowShelf(osRate, 150.0, 0.707f, getG("bass"));
    
    *postEQ.template get<MiddleIndex>().coefficients = *FilterCoefs::makePeakFilter(osRate, getVal("frequency"), 0.5f, getG("middle"));
    
    *postEQ.template get<TrebleIndex>().coefficients = *FilterCoefs::makeHighShelf(osRate, 3000.0, 0.707f, getG("treble"));
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    // 1. Oversampling Up
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> osBlock = oversampler->processSamplesUp(block);

    // 2. Distortion (In Oversampled domain)
    updateFilters();

    float gainAmt = apvts.getRawParameterValue("gain")->load();
    for (size_t ch = 0; ch < osBlock.getNumChannels(); ++ch)
    {
        float* data = osBlock.getChannelPointer(ch);
        for (size_t s = 0; s < osBlock.getNumSamples(); ++s)
            data[s] = std::tanh(data[s] * gainAmt);
    }

    postEQ.process(juce::dsp::ProcessContextReplacing<float>(osBlock));

    // 4. Oversampling Down
    oversampler->processSamplesDown(block);

    // 5. Final Output Gain
    buffer.applyGain(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("out")->load()));

    rmsLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
}

// Boilers (Required for linking)
const juce::String AudioPluginAudioProcessor::getName() const { return "Garrotxa"; }
bool AudioPluginAudioProcessor::acceptsMidi() const { return false; }
bool AudioPluginAudioProcessor::producesMidi() const { return false; }
double AudioPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int AudioPluginAudioProcessor::getNumPrograms() { return 1; }
int AudioPluginAudioProcessor::getCurrentProgram() { return 0; }
void AudioPluginAudioProcessor::setCurrentProgram (int index) { juce::ignoreUnused(index); }
const juce::String AudioPluginAudioProcessor::getProgramName (int index) { juce::ignoreUnused(index); return {}; }
void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }
bool AudioPluginAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() { return new AudioPluginAudioProcessorEditor (*this); }
void AudioPluginAudioProcessor::releaseResources() {}
bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const { juce::ignoreUnused(layouts); return true; }

void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AudioPluginAudioProcessor(); }