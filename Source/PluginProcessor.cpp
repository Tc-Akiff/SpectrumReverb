#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr std::array<const char*, 10> parameterIDs {
    "predelay", "decay", "size", "damping", "mix",
    "diffusion", "modulation", "color", "width", "output"
};

struct FactoryPreset
{
    std::array<float, 10> values;
};

constexpr std::array<FactoryPreset, 5> factoryPresets {{
    { { 12.0f, 2.8f, 0.65f, 0.45f, 0.40f, 0.75f, 0.18f, 0.55f, 0.90f, 0.0f } },
    { { 28.0f, 3.8f, 0.72f, 0.34f, 0.32f, 0.82f, 0.22f, 0.72f, 0.88f, 0.0f } },
    { { 6.0f, 2.2f, 0.58f, 0.68f, 0.36f, 0.90f, 0.14f, 0.35f, 0.78f, 0.0f } },
    { { 34.0f, 5.6f, 0.84f, 0.48f, 0.42f, 0.72f, 0.31f, 0.64f, 1.0f, 0.0f } },
    { { 42.0f, 12.0f, 1.0f, 0.26f, 0.48f, 0.95f, 0.52f, 0.78f, 1.0f, -2.0f } }
}};
}

SpectraReverbAudioProcessor::SpectraReverbAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "SPECTRA_PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SpectraReverbAudioProcessor::createParameterLayout()
{
    using FloatParameter = juce::AudioParameterFloat;
    using Range = juce::NormalisableRange<float>;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "predelay", 1 }, "Pre-delay", Range(0.0f, 250.0f, 0.1f), 12.0f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "decay", 1 }, "Decay Time", Range(0.2f, 20.0f, 0.01f, 0.35f), 2.8f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "size", 1 }, "Size", Range(0.0f, 1.0f, 0.001f), 0.65f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "damping", 1 }, "Damping", Range(0.0f, 1.0f, 0.001f), 0.45f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "mix", 1 }, "Mix", Range(0.0f, 1.0f, 0.001f), 0.40f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "diffusion", 1 }, "Diffusion", Range(0.0f, 1.0f, 0.001f), 0.75f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "modulation", 1 }, "Modulation", Range(0.0f, 1.0f, 0.001f), 0.18f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "color", 1 }, "Color", Range(0.0f, 1.0f, 0.001f), 0.55f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "width", 1 }, "Width", Range(0.0f, 1.0f, 0.001f), 0.90f));
    layout.add(std::make_unique<FloatParameter>(
        juce::ParameterID { "output", 1 }, "Output", Range(-24.0f, 12.0f, 0.1f), 0.0f));
    return layout;
}

const juce::String SpectraReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectraReverbAudioProcessor::acceptsMidi() const
{
    return false;
}

bool SpectraReverbAudioProcessor::producesMidi() const
{
    return false;
}

bool SpectraReverbAudioProcessor::isMidiEffect() const
{
    return false;
}

double SpectraReverbAudioProcessor::getTailLengthSeconds() const
{
    return 20.0;
}

int SpectraReverbAudioProcessor::getNumPrograms()
{
    return static_cast<int>(factoryPresets.size());
}

int SpectraReverbAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void SpectraReverbAudioProcessor::setCurrentProgram(int index)
{
    applyFactoryPreset(index);
}

const juce::String SpectraReverbAudioProcessor::getProgramName(int index)
{
    static const std::array<const char*, 5> names {
        "Init - Clear", "Vocal Hall", "Dark Plate", "Wide Ambience", "Cosmic Bloom"
    };
    return juce::isPositiveAndBelow(index, static_cast<int>(names.size())) ? names[static_cast<std::size_t>(index)] : "";
}

void SpectraReverbAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void SpectraReverbAudioProcessor::prepareToPlay(double sampleRate, int)
{
    reverb.prepare(sampleRate);

    for (std::size_t i = 0; i < parameterIDs.size(); ++i)
    {
        const auto* raw = parameters.getRawParameterValue(parameterIDs[i]);
        const auto value = raw != nullptr ? raw->load() : 0.0f;
        smoothedParameters[i].reset(sampleRate, 0.02);
        smoothedParameters[i].setCurrentAndTargetValue(value);
    }
}

void SpectraReverbAudioProcessor::releaseResources()
{
    reverb.reset();
}

bool SpectraReverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    const auto supportedInput = input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo();
    const auto supportedOutput = output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
    return supportedInput && supportedOutput;
}

void SpectraReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto inputChannels = getTotalNumInputChannels();
    const auto outputChannels = getTotalNumOutputChannels();
    const auto sampleCount = buffer.getNumSamples();
    for (auto channel = inputChannels; channel < outputChannels; ++channel)
        buffer.clear(channel, 0, sampleCount);

    if (inputChannels == 0 || outputChannels == 0)
        return;

    auto* outputLeft = buffer.getWritePointer(0);
    auto* outputRight = outputChannels > 1 ? buffer.getWritePointer(1) : nullptr;
    const auto* inputLeft = buffer.getReadPointer(0);
    const auto* inputRight = inputChannels > 1 ? buffer.getReadPointer(1) : inputLeft;

    std::array<float, 10> targets {};
    for (std::size_t i = 0; i < parameterIDs.size(); ++i)
    {
        if (const auto* raw = parameters.getRawParameterValue(parameterIDs[i]))
            targets[i] = raw->load();
        smoothedParameters[i].setTargetValue(targets[i]);
    }

    float peakInputLeft = 0.0f;
    float peakInputRight = 0.0f;
    float peakOutputLeft = 0.0f;
    float peakOutputRight = 0.0f;

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        const auto inL = inputLeft[sample];
        const auto inR = inputRight[sample];
        ReverbParameters current;
        current.preDelayMs = smoothedParameters[0].getNextValue();
        current.decaySeconds = smoothedParameters[1].getNextValue();
        current.size = smoothedParameters[2].getNextValue();
        current.damping = smoothedParameters[3].getNextValue();
        current.mix = smoothedParameters[4].getNextValue();
        current.diffusion = smoothedParameters[5].getNextValue();
        current.modulation = smoothedParameters[6].getNextValue();
        current.color = smoothedParameters[7].getNextValue();
        current.width = smoothedParameters[8].getNextValue();
        current.outputDb = smoothedParameters[9].getNextValue();

        const auto out = reverb.processSample(inL, inR, current);
        if (outputRight != nullptr)
        {
            outputLeft[sample] = out.left;
            outputRight[sample] = out.right;
        }
        else
        {
            outputLeft[sample] = 0.5f * (out.left + out.right);
        }

        peakInputLeft = std::max(peakInputLeft, std::abs(inL));
        peakInputRight = std::max(peakInputRight, std::abs(inR));
        peakOutputLeft = std::max(peakOutputLeft, std::abs(outputLeft[sample]));
        peakOutputRight = std::max(
            peakOutputRight, std::abs(outputRight != nullptr ? outputRight[sample] : outputLeft[sample]));
    }

    inputMeterLeft.store(peakInputLeft);
    inputMeterRight.store(peakInputRight);
    outputMeterLeft.store(peakOutputLeft);
    outputMeterRight.store(peakOutputRight);
}

juce::AudioProcessorEditor* SpectraReverbAudioProcessor::createEditor()
{
    return new SpectraReverbAudioProcessorEditor(*this);
}

bool SpectraReverbAudioProcessor::hasEditor() const
{
    return true;
}

void SpectraReverbAudioProcessor::setParameterValue(const juce::String& parameterID, float value)
{
    if (auto* parameter = parameters.getParameter(parameterID))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}

void SpectraReverbAudioProcessor::applyFactoryPreset(int index)
{
    if (!juce::isPositiveAndBelow(index, static_cast<int>(factoryPresets.size())))
        return;

    const auto& values = factoryPresets[static_cast<std::size_t>(index)].values;
    for (std::size_t i = 0; i < parameterIDs.size(); ++i)
        setParameterValue(parameterIDs[i], values[i]);
    currentProgram = index;
}

void SpectraReverbAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData)
{
    auto state = parameters.copyState();
    state.setProperty("factoryProgram", currentProgram, nullptr);
    if (const auto xml = state.createXml())
        copyXmlToBinary(*xml, destinationData);
}

void SpectraReverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(parameters.state.getType()))
        {
            const auto state = juce::ValueTree::fromXml(*xml);
            parameters.replaceState(state);
            const auto savedProgram = static_cast<int>(state.getProperty("factoryProgram", 0));
            if (juce::isPositiveAndBelow(savedProgram, static_cast<int>(factoryPresets.size())))
                currentProgram = savedProgram;
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectraReverbAudioProcessor();
}
