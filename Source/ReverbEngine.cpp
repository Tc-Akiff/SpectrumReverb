#include "ReverbEngine.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr std::array<float, 8> delayTimesMs { 29.7f, 33.1f, 38.3f, 41.9f, 47.3f, 53.1f, 59.3f, 62.3f };
constexpr std::array<float, 8> modulationRates { 0.071f, 0.083f, 0.097f, 0.109f, 0.127f, 0.139f, 0.157f, 0.173f };
constexpr std::array<float, 8> leftSigns { 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
constexpr std::array<float, 8> rightSigns { 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f };
constexpr std::array<float, 4> diffuserTimesMs { 2.3f, 3.8f, 5.1f, 7.7f };
constexpr double twoPi = 6.28318530717958647692;
}

void ReverbEngine::AllpassLine::prepare(std::size_t delay)
{
    delaySamples = std::max<std::size_t>(1, delay);
    samples.assign(delaySamples + 1, 0.0f);
    writePosition = 0;
}

void ReverbEngine::AllpassLine::reset()
{
    std::fill(samples.begin(), samples.end(), 0.0f);
    writePosition = 0;
}

float ReverbEngine::AllpassLine::process(float input, float coefficient)
{
    const auto readPosition = (writePosition + samples.size() - delaySamples) % samples.size();
    const auto delayed = samples[readPosition];
    const auto output = delayed - coefficient * input;
    samples[writePosition] = input + coefficient * output;
    writePosition = (writePosition + 1) % samples.size();
    return output;
}

void ReverbEngine::prepare(double newSampleRate)
{
    sampleRate = std::max(8000.0, newSampleRate);

    const auto maximumDelaySamples = static_cast<std::size_t>(std::ceil(sampleRate * 0.15)) + 8;
    for (auto& line : delayLines)
    {
        line.samples.assign(maximumDelaySamples, 0.0f);
        line.writePosition = 0;
    }

    for (std::size_t channel = 0; channel < diffusers.size(); ++channel)
    {
        for (std::size_t stage = 0; stage < diffuserCount; ++stage)
        {
            const auto channelOffsetMs = channel == 0 ? 0.0f : 0.61f;
            const auto delay = static_cast<std::size_t>(
                std::round((diffuserTimesMs[stage] + channelOffsetMs) * sampleRate / 1000.0));
            diffusers[channel][stage].prepare(delay);
        }
    }

    const auto preDelaySize = static_cast<std::size_t>(std::ceil(sampleRate * 0.26)) + 2;
    for (auto& buffer : preDelayBuffers)
        buffer.assign(preDelaySize, 0.0f);

    reset();
}

void ReverbEngine::reset()
{
    for (auto& line : delayLines)
    {
        std::fill(line.samples.begin(), line.samples.end(), 0.0f);
        line.writePosition = 0;
        line.dampingState = 0.0f;
        line.phase = 0.0;
    }

    for (auto& channel : diffusers)
        for (auto& diffuser : channel)
            diffuser.reset();

    for (auto& buffer : preDelayBuffers)
        std::fill(buffer.begin(), buffer.end(), 0.0f);

    preDelayWritePosition = 0;
    inputColorLeft = 0.0f;
    inputColorRight = 0.0f;
    cachedColor = -1.0f;
    cachedDamping = -1.0f;
    cachedDecay = -1.0f;
    cachedSize = -1.0f;
    cachedMix = -1.0f;
    cachedOutputDb = -100.0f;
}

float ReverbEngine::readFractional(const std::vector<float>& buffer, std::size_t writePosition, double delaySamples)
{
    if (buffer.empty())
        return 0.0f;

    const auto size = static_cast<double>(buffer.size());
    auto readPosition = static_cast<double>(writePosition) - delaySamples;
    readPosition = std::fmod(readPosition, size);
    if (readPosition < 0.0)
        readPosition += size;

    const auto index0 = static_cast<std::size_t>(readPosition);
    const auto index1 = (index0 + 1) % buffer.size();
    const auto fraction = static_cast<float>(readPosition - static_cast<double>(index0));
    return buffer[index0] + fraction * (buffer[index1] - buffer[index0]);
}

void ReverbEngine::updateToneCoefficients(const ReverbParameters& parameters)
{
    if (std::abs(parameters.color - cachedColor) > 0.001f)
    {
        const auto cutoffHz = 1800.0f + 17800.0f * parameters.color;
        colorCoefficient = 1.0f - std::exp(-static_cast<float>(twoPi) * cutoffHz / static_cast<float>(sampleRate));
        cachedColor = parameters.color;
    }

    if (std::abs(parameters.damping - cachedDamping) > 0.001f)
    {
        const auto cutoffHz = 18000.0f * std::pow(0.1f, parameters.damping);
        dampingCoefficient = 1.0f - std::exp(-static_cast<float>(twoPi) * cutoffHz / static_cast<float>(sampleRate));
        cachedDamping = parameters.damping;
    }
}

void ReverbEngine::updateFeedbackGains(const ReverbParameters& parameters)
{
    const auto sizeScale = 0.55f + 1.10f * parameters.size;
    const auto decay = std::max(0.2f, parameters.decaySeconds);

    for (std::size_t i = 0; i < lineCount; ++i)
    {
        const auto delaySeconds = delayTimesMs[i] * sizeScale / 1000.0f;
        feedbackGains[i] = std::pow(10.0f, -3.0f * delaySeconds / decay);
    }

    cachedDecay = parameters.decaySeconds;
    cachedSize = parameters.size;
}

StereoSample ReverbEngine::processSample(float inputLeft, float inputRight, const ReverbParameters& parameters)
{
    updateToneCoefficients(parameters);
    if (std::abs(parameters.decaySeconds - cachedDecay) > 0.002f
        || std::abs(parameters.size - cachedSize) > 0.001f)
        updateFeedbackGains(parameters);

    if (std::abs(parameters.mix - cachedMix) > 0.001f)
    {
        wetMixGain = std::sin(parameters.mix * static_cast<float>(twoPi) * 0.25f);
        dryMixGain = std::cos(parameters.mix * static_cast<float>(twoPi) * 0.25f);
        cachedMix = parameters.mix;
    }

    if (std::abs(parameters.outputDb - cachedOutputDb) > 0.001f)
    {
        outputGainLinear = std::pow(10.0f, parameters.outputDb / 20.0f);
        cachedOutputDb = parameters.outputDb;
    }

    inputColorLeft += colorCoefficient * (inputLeft - inputColorLeft);
    inputColorRight += colorCoefficient * (inputRight - inputColorRight);

    auto diffusedLeft = inputColorLeft;
    auto diffusedRight = inputColorRight;
    const auto diffusionCoefficient = 0.10f + 0.75f * parameters.diffusion;
    for (std::size_t stage = 0; stage < diffuserCount; ++stage)
    {
        diffusedLeft = diffusers[0][stage].process(diffusedLeft, diffusionCoefficient);
        diffusedRight = diffusers[1][stage].process(diffusedRight, diffusionCoefficient);
    }

    const auto preDelaySamples = std::clamp(
        static_cast<double>(parameters.preDelayMs) * sampleRate / 1000.0,
        0.0,
        static_cast<double>(preDelayBuffers[0].size() - 2));
    preDelayBuffers[0][preDelayWritePosition] = diffusedLeft;
    preDelayBuffers[1][preDelayWritePosition] = diffusedRight;
    const auto delayedLeft = readFractional(preDelayBuffers[0], preDelayWritePosition, preDelaySamples);
    const auto delayedRight = readFractional(preDelayBuffers[1], preDelayWritePosition, preDelaySamples);
    preDelayWritePosition = (preDelayWritePosition + 1) % preDelayBuffers[0].size();

    std::array<float, lineCount> delayed {};
    float feedbackSum = 0.0f;
    const auto sizeScale = 0.55f + 1.10f * parameters.size;
    const auto modulationDepth = static_cast<double>(parameters.modulation) * sampleRate * 0.0015;

    for (std::size_t i = 0; i < lineCount; ++i)
    {
        auto& line = delayLines[i];
        const auto modulation = std::sin(line.phase) * modulationDepth;
        const auto delay = std::clamp(
            delayTimesMs[i] * sizeScale * sampleRate / 1000.0 + modulation,
            1.0,
            static_cast<double>(line.samples.size() - 2));
        const auto sample = readFractional(line.samples, line.writePosition, delay);
        line.dampingState += dampingCoefficient * (sample - line.dampingState);
        delayed[i] = line.dampingState;
        feedbackSum += delayed[i];

        line.phase += twoPi * modulationRates[i] / sampleRate;
        if (line.phase >= twoPi)
            line.phase -= twoPi;
    }

    constexpr float injectionGain = 0.176776695f;
    constexpr float outputGain = 0.353553391f;
    float wetLeft = 0.0f;
    float wetRight = 0.0f;

    for (std::size_t i = 0; i < lineCount; ++i)
    {
        const auto householder = delayed[i] - 0.25f * feedbackSum;
        const auto injection = injectionGain * (delayedLeft * leftSigns[i] + delayedRight * rightSigns[i]);
        auto& line = delayLines[i];
        line.samples[line.writePosition] = injection + feedbackGains[i] * householder;
        line.writePosition = (line.writePosition + 1) % line.samples.size();

        wetLeft += delayed[i] * leftSigns[i] * outputGain;
        wetRight += delayed[i] * rightSigns[i] * outputGain;
    }

    const auto middle = 0.5f * (wetLeft + wetRight);
    const auto side = 0.5f * (wetLeft - wetRight) * parameters.width;
    wetLeft = middle + side;
    wetRight = middle - side;

    return {
        (inputLeft * dryMixGain + wetLeft * wetMixGain) * outputGainLinear,
        (inputRight * dryMixGain + wetRight * wetMixGain) * outputGainLinear
    };
}
