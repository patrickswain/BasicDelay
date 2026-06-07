/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicDelayAudioProcessor::BasicDelayAudioProcessor() : apvts(*this, nullptr, "Parameters", createParameterLayout())
#ifndef JucePlugin_PreferredChannelConfigurations
     , AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

BasicDelayAudioProcessor::~BasicDelayAudioProcessor()
{

}

//==============================================================================
const juce::String BasicDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    bufferSize = 2 * (int)sampleRate;
    
    delayBuffer.assign(bufferSize, 0.0f);

    ParamSettings tempSettings = getParamSettings(apvts);
    resetDelay();
    cookVariables();
    return;
}

void BasicDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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

void BasicDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    float xn;
    float yn;

    ParamSettings settings = getParamSettings(apvts);

    auto leftChannel = buffer.getWritePointer(0);
    auto rightChannel = buffer.getWritePointer(1);
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    for (auto sample = 0; sample < numSamples; ++sample)
    {
        jassert(writeIndex < bufferSize);
        jassert(readIndex < bufferSize);

        xn = leftChannel[sample];
        yn = delayBuffer[readIndex];
        delayBuffer[writeIndex] = xn + (yn * feedback);

        leftChannel[sample] = (xn * dryMix) + (yn * wetMix);

        readIndex++;
        writeIndex++; 
        if (readIndex >= bufferSize)
            readIndex = 0;
        if (writeIndex >= bufferSize)
            writeIndex = 0;
    }
     
}

//==============================================================================
bool BasicDelayAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicDelayAudioProcessor::createEditor()
{
    return new BasicDelayAudioProcessorEditor (*this);
}

//==============================================================================
void BasicDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void BasicDelayAudioProcessor::resetDelay ()
{
    std::fill(delayBuffer.begin(), delayBuffer.end(), 0);
    writeIndex = 0;
    readIndex = 0;
}

void BasicDelayAudioProcessor::cookVariables()
{
    readIndex = writeIndex - (int)delayInSamples; 
    if (readIndex < 0)
        readIndex += bufferSize;
    dryMix = wetMix - 1;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicDelayAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout BasicDelayAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::wetMix, ParamNames::wetMix, ParamRanges::wetMix, ParamDefaultValues::wetMix));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::delayInSamples, ParamNames::delayInSamples, ParamRanges::delayInSamples, ParamDefaultValues::delayInSamples));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::feedback, ParamNames::feedback, ParamRanges::feedback, ParamDefaultValues::feedback));
    return layout;
}

ParamSettings BasicDelayAudioProcessor::getParamSettings(juce::AudioProcessorValueTreeState& tree)
{
    ParamSettings settings;


    wetMix = tree.getRawParameterValue(ParamIDs::wetMix)->load();
    delayInSamples = tree.getRawParameterValue(ParamIDs::delayInSamples)->load();
    //settings.delayInBPM = apvts.getRawParameterValue(ParamIDs::delayInBPM)->load();
    feedback = tree.getRawParameterValue(ParamIDs::feedback)->load();

    cookVariables();

    return settings;
}