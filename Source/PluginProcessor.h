#pragma once

#include <JuceHeader.h>
#include "ReverbEngine.h"

#include <array>
#include <atomic>

class SpectraReverbAudioProcessor final : public juce::AudioProcessor
{
public:
    SpectraReverbAudioProcessor();
    ~SpectraReverbAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
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

    void getStateInformation(juce::MemoryBlock& destinationData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void applyFactoryPreset(int index);

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> inputMeterLeft { 0.0f };
    std::atomic<float> inputMeterRight { 0.0f };
    std::atomic<float> outputMeterLeft { 0.0f };
    std::atomic<float> outputMeterRight { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void setParameterValue(const juce::String& parameterID, float value);

    ReverbEngine reverb;
    std::array<juce::SmoothedValue<float>, 10> smoothedParameters;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectraReverbAudioProcessor)
};
