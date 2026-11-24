#pragma once

#include <array>
#include <cstddef>
#include <vector>

struct ReverbParameters
{
    float preDelayMs = 12.0f;
    float decaySeconds = 2.8f;
    float size = 0.65f;
    float damping = 0.45f;
    float mix = 0.40f;
    float diffusion = 0.75f;
    float modulation = 0.18f;
    float color = 0.55f;
    float width = 0.90f;
    float outputDb = 0.0f;
};

struct StereoSample
{
    float left = 0.0f;
    float right = 0.0f;
};

class ReverbEngine
{
public:
    void prepare(double sampleRate);
    void reset();
    StereoSample processSample(float inputLeft, float inputRight, const ReverbParameters& parameters);

private:
    static constexpr std::size_t lineCount = 8;
    static constexpr std::size_t diffuserCount = 4;

    struct DelayLine
    {
        std::vector<float> samples;
        std::size_t writePosition = 0;
        float dampingState = 0.0f;
        double phase = 0.0;
    };

    struct AllpassLine
    {
        std::vector<float> samples;
        std::size_t writePosition = 0;
        std::size_t delaySamples = 1;

        void prepare(std::size_t delay);
        void reset();
        float process(float input, float coefficient);
    };

    static float readFractional(const std::vector<float>& buffer, std::size_t writePosition, double delaySamples);
    void updateToneCoefficients(const ReverbParameters& parameters);
    void updateFeedbackGains(const ReverbParameters& parameters);

    double sampleRate = 44100.0;
    std::array<DelayLine, lineCount> delayLines;
    std::array<std::array<AllpassLine, diffuserCount>, 2> diffusers;
    std::array<std::vector<float>, 2> preDelayBuffers;
    std::size_t preDelayWritePosition = 0;

    float inputColorLeft = 0.0f;
    float inputColorRight = 0.0f;
    float colorCoefficient = 1.0f;
    float dampingCoefficient = 1.0f;
    float cachedColor = -1.0f;
    float cachedDamping = -1.0f;
    float cachedDecay = -1.0f;
    float cachedSize = -1.0f;
    float cachedMix = -1.0f;
    float cachedOutputDb = -100.0f;
    float wetMixGain = 0.0f;
    float dryMixGain = 1.0f;
    float outputGainLinear = 1.0f;
    std::array<float, lineCount> feedbackGains {};
};
