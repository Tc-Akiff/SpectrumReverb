#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include <array>
#include <memory>

class SpectraLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    SpectraLookAndFeel();

    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosition, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosition, float minSliderPosition, float maxSliderPosition,
                          juce::Slider::SliderStyle, juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;
};

class SpectraReverbAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                private juce::Timer
{
public:
    explicit SpectraReverbAudioProcessorEditor(SpectraReverbAudioProcessor&);
    ~SpectraReverbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    juce::String formatValue(std::size_t index) const;
    void updateMeterHistory();
    juce::Slider* controlFor(std::size_t index) const;

    SpectraReverbAudioProcessor& audioProcessor;
    SpectraLookAndFeel lookAndFeel;
    std::array<std::unique_ptr<juce::Slider>, 5> mainKnobs;
    std::array<std::unique_ptr<juce::Slider>, 5> detailSliders;
    std::array<std::unique_ptr<juce::Label>, 10> controlLabels;
    std::array<std::unique_ptr<juce::Label>, 10> valueLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 10> attachments;
    juce::ComboBox presetBox;
    std::array<float, 128> waveform {};
    int waveformPosition = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectraReverbAudioProcessorEditor)
};
