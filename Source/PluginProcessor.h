/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct ParamIDs
{
    static constexpr auto wetMix = "wetMix";
    static constexpr auto delayInBPM = "delayInBPM";
    static constexpr auto delayInSamples = "delayInSamples";
    static constexpr auto feedback = "feedback";
};

struct ParamNames
{
    static constexpr auto wetMix = "Wet Mix";
    static constexpr auto delayInBPM = "Delay (BPM)";
    //static constexpr auto delayInSamples = "Delay (Samples)";
    static constexpr auto feedback = "Feedback";
};

struct ParamSettings
{
    float wetMix = { 0.5f };
    //float delayInSamples = { 20000.0f };
    int delayInBPM = { 60 };
    float feedback = { 0.5f };
};

struct ParamRanges
{
    inline static const juce::NormalisableRange<float> wetMix = { 0.0f, 1.0f };
    //inline static const juce::NormalisableRange<float> delayInSamples = { 0.0f, 44100.0f };
    inline static const juce::NormalisableRange<int> delayInBPM = { 1, 200 };
    inline static const juce::NormalisableRange<float> feedback = { -1.0f, 1.0f };
};

struct ParamDefaultValues
{
    static constexpr float wetMix = { -20.0f };
    static constexpr int delayInBPM = { 120 };
    static constexpr float delayInSamples = { 10000.0f };
    static constexpr float feedback = { -20.0f };
};


//==============================================================================
/**
*/
class BasicDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    BasicDelayAudioProcessor();
    ~BasicDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;
    ParamSettings getParamSettings(juce::AudioProcessorValueTreeState& apvts);
    //==============================================================================
    void resetDelay();
    void cookVariables();
private:
    float wetMix;
    float dryMix;
    float feedback;

    std::vector<float> delayBuffer;
    float delayInSamples;
    int readIndex;
    int writeIndex;
    int bufferSize;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicDelayAudioProcessor)
};
