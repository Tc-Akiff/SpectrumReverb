#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr std::array<const char*, 10> parameterIDs {
    "predelay", "decay", "size", "damping", "mix",
    "diffusion", "modulation", "color", "width", "output"
};

constexpr std::array<const char*, 10> controlNames {
    "PRE-DELAY", "DECAY TIME", "SIZE", "DAMPING", "MIX",
    "DIFFUSION", "MODULATION", "COLOR", "WIDTH", "OUTPUT"
};

constexpr std::array<const char*, 5> presetNames {
    "Init - Clear", "Vocal Hall", "Dark Plate", "Wide Ambience", "Cosmic Bloom"
};

juce::Colour orange()
{
    return juce::Colour(0xffff9a35);
}
}

SpectraLookAndFeel::SpectraLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xffdce0e2));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xff263038));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff929ba0));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xffe6e9e9));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xff263038));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, orange());
}

juce::Font SpectraLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(14.0f, juce::Font::plain);
}

void SpectraLookAndFeel::drawRotarySlider(juce::Graphics& graphics, int x, int y, int width, int height,
                                          float sliderPosition, float, float, juce::Slider&)
{
    const auto diameter = static_cast<float>(std::min(width, height) - 16);
    const auto radius = diameter * 0.5f;
    const auto centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const auto centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const auto startAngle = juce::MathConstants<float>::pi * 1.25f;
    const auto endAngle = juce::MathConstants<float>::pi * 2.75f;
    const auto angle = startAngle + sliderPosition * (endAngle - startAngle);
    const auto arcRadius = radius + 3.0f;

    juce::Path track;
    track.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                        startAngle, endAngle, true);
    graphics.setColour(juce::Colour(0xff9ca4a7));
    graphics.strokePath(track, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path active;
    active.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                         startAngle, std::max(startAngle + 0.001f, angle), true);
    graphics.setColour(orange());
    graphics.strokePath(active, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto body = juce::Rectangle<float>(centreX - radius, centreY - radius, diameter, diameter);
    graphics.setColour(juce::Colour(0x35000000));
    graphics.fillEllipse(body.translated(0.0f, 4.0f));

    juce::ColourGradient face(juce::Colour(0xfff7f8f8), centreX - radius, centreY - radius,
                              juce::Colour(0xffaeb7bb), centreX + radius, centreY + radius, true);
    graphics.setGradientFill(face);
    graphics.fillEllipse(body);
    graphics.setColour(juce::Colour(0xfffdfefe));
    graphics.drawEllipse(body, 2.0f);
    graphics.setColour(juce::Colour(0xffa5adb0));
    graphics.drawEllipse(body.reduced(7.0f), 1.0f);

    const auto pointerRadius = radius * 0.70f;
    const auto pointerX = centreX + std::cos(angle - juce::MathConstants<float>::halfPi) * pointerRadius;
    const auto pointerY = centreY + std::sin(angle - juce::MathConstants<float>::halfPi) * pointerRadius;
    graphics.setColour(orange());
    graphics.fillEllipse(pointerX - 4.0f, pointerY - 4.0f, 8.0f, 8.0f);
}

void SpectraLookAndFeel::drawLinearSlider(juce::Graphics& graphics, int x, int y, int width, int height,
                                          float sliderPosition, float, float,
                                          juce::Slider::SliderStyle style, juce::Slider&)
{
    if (style != juce::Slider::LinearHorizontal)
        return;

    const auto trackY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const auto left = static_cast<float>(x) + 8.0f;
    const auto right = static_cast<float>(x + width) - 8.0f;
    const auto position = std::clamp(sliderPosition, left, right);

    graphics.setColour(juce::Colour(0xff858e92));
    graphics.fillRoundedRectangle(left, trackY - 4.0f, right - left, 8.0f, 4.0f);
    graphics.setColour(juce::Colour(0xfff1f3f3));
    graphics.fillRoundedRectangle(left, trackY - 2.0f, right - left, 4.0f, 2.0f);
    graphics.setColour(orange());
    graphics.fillRoundedRectangle(left, trackY - 3.0f, position - left, 6.0f, 3.0f);

    graphics.setColour(juce::Colour(0xfff8f9f9));
    graphics.fillRoundedRectangle(position - 9.0f, trackY - 10.0f, 18.0f, 20.0f, 6.0f);
    graphics.setColour(orange());
    graphics.drawRoundedRectangle(position - 9.0f, trackY - 10.0f, 18.0f, 20.0f, 6.0f, 2.0f);
}

void SpectraLookAndFeel::drawComboBox(juce::Graphics& graphics, int width, int height, bool,
                                      int, int, int, int, juce::ComboBox&)
{
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    graphics.setColour(juce::Colour(0x22000000));
    graphics.fillRoundedRectangle(bounds.translated(0.0f, 2.0f), 18.0f);
    juce::ColourGradient fill(juce::Colour(0xfff5f6f6), 0.0f, 0.0f,
                              juce::Colour(0xffb7c0c3), 0.0f, static_cast<float>(height), false);
    graphics.setGradientFill(fill);
    graphics.fillRoundedRectangle(bounds, 18.0f);
    graphics.setColour(juce::Colour(0xffa0a9ad));
    graphics.drawRoundedRectangle(bounds, 18.0f, 1.0f);
    graphics.setColour(juce::Colour(0xff354047));
    juce::Path arrow;
    const auto centreX = static_cast<float>(width) - 18.0f;
    const auto centreY = static_cast<float>(height) * 0.5f;
    arrow.startNewSubPath(centreX - 4.0f, centreY - 2.0f);
    arrow.lineTo(centreX, centreY + 2.0f);
    arrow.lineTo(centreX + 4.0f, centreY - 2.0f);
    graphics.strokePath(arrow, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

SpectraReverbAudioProcessorEditor::SpectraReverbAudioProcessorEditor(SpectraReverbAudioProcessor& processor)
    : AudioProcessorEditor(&processor), audioProcessor(processor)
{
    setLookAndFeel(&lookAndFeel);
    setSize(1120, 620);

    for (std::size_t i = 0; i < controlLabels.size(); ++i)
    {
        const auto isMain = i < mainKnobs.size();
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(isMain ? juce::Slider::RotaryHorizontalVerticalDrag : juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider->setLookAndFeel(&lookAndFeel);

        auto label = std::make_unique<juce::Label>();
        label->setText(controlNames[i], juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colour(0xff303a40));
        label->setFont(juce::Font(15.0f, juce::Font::plain));

        auto value = std::make_unique<juce::Label>();
        value->setJustificationType(juce::Justification::centred);
        value->setColour(juce::Label::textColourId, juce::Colour(0xff515b61));
        value->setFont(juce::Font(16.0f, juce::Font::plain));

        addAndMakeVisible(*slider);
        addAndMakeVisible(*label);
        addAndMakeVisible(*value);

        if (isMain)
            mainKnobs[i] = std::move(slider);
        else
            detailSliders[i - mainKnobs.size()] = std::move(slider);
        controlLabels[i] = std::move(label);
        valueLabels[i] = std::move(value);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, parameterIDs[i], *controlFor(i));
    }

    presetBox.setLookAndFeel(&lookAndFeel);
    presetBox.setJustificationType(juce::Justification::centredLeft);
    for (std::size_t i = 0; i < presetNames.size(); ++i)
        presetBox.addItem(presetNames[i], static_cast<int>(i + 1));
    presetBox.setSelectedId(1, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        const auto selected = presetBox.getSelectedId();
        if (selected > 0)
            audioProcessor.applyFactoryPreset(selected - 1);
    };
    addAndMakeVisible(presetBox);

    startTimerHz(30);
}

SpectraReverbAudioProcessorEditor::~SpectraReverbAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
    presetBox.setLookAndFeel(nullptr);
}

juce::Slider* SpectraReverbAudioProcessorEditor::controlFor(std::size_t index) const
{
    return index < mainKnobs.size() ? mainKnobs[index].get() : detailSliders[index - mainKnobs.size()].get();
}

void SpectraReverbAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    const auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient background(juce::Colour(0xffedf0f1), 0.0f, 0.0f,
                                    juce::Colour(0xff626d72), static_cast<float>(getWidth()), static_cast<float>(getHeight()), false);
    graphics.setGradientFill(background);
    graphics.fillRoundedRectangle(bounds.reduced(8.0f), 16.0f);
    graphics.setColour(juce::Colour(0xff343b3e));
    graphics.drawRoundedRectangle(bounds.reduced(8.0f), 16.0f, 8.0f);

    graphics.setColour(juce::Colour(0xffb5bdc0));
    graphics.drawHorizontalLine(100, 34.0f, static_cast<float>(getWidth() - 34));
    graphics.drawHorizontalLine(405, 34.0f, static_cast<float>(getWidth() - 34));

    graphics.setColour(juce::Colour(0xffaeb7bb));
    graphics.setFont(juce::Font(29.0f, juce::Font::plain));
    graphics.drawText("SPECTRA REVERB", 38, 30, 370, 45, juce::Justification::centredLeft);
    graphics.setColour(orange());
    graphics.fillEllipse(416.0f, 47.0f, 10.0f, 10.0f);
    graphics.setColour(juce::Colour(0x55ff9a35));
    graphics.fillEllipse(412.0f, 43.0f, 18.0f, 18.0f);

    const auto meterBounds = juce::Rectangle<float>(840.0f, 122.0f, 240.0f, 244.0f);
    graphics.setColour(juce::Colour(0x33000000));
    graphics.fillRoundedRectangle(meterBounds.translated(0.0f, 4.0f), 24.0f);
    juce::ColourGradient meterFill(juce::Colour(0xfff4f6f6), 840.0f, 122.0f,
                                   juce::Colour(0xff899397), 1080.0f, 366.0f, false);
    graphics.setGradientFill(meterFill);
    graphics.fillRoundedRectangle(meterBounds, 24.0f);
    graphics.setColour(juce::Colour(0xfff7f8f8));
    graphics.drawRoundedRectangle(meterBounds, 24.0f, 2.0f);
    graphics.setColour(juce::Colour(0xff303a40));
    graphics.setFont(juce::Font(17.0f, juce::Font::plain));
    graphics.drawText("INPUT / OUTPUT", meterBounds.toNearestInt().removeFromTop(38), juce::Justification::centred);

    const auto display = juce::Rectangle<float>(880.0f, 168.0f, 160.0f, 174.0f);
    graphics.setColour(juce::Colour(0xff727c80));
    graphics.fillRoundedRectangle(display, 14.0f);
    graphics.setColour(juce::Colour(0xffcbd1d3));
    graphics.fillRoundedRectangle(display.reduced(3.0f), 12.0f);

    juce::Path waveformPath;
    const auto midY = display.getCentreY();
    const auto amplitude = display.getHeight() * 0.39f;
    for (std::size_t i = 0; i < waveform.size(); ++i)
    {
        const auto x = display.getX() + 8.0f + static_cast<float>(i) * (display.getWidth() - 16.0f)
            / static_cast<float>(waveform.size() - 1);
        const auto y = midY - waveform[i] * amplitude;
        if (i == 0)
            waveformPath.startNewSubPath(x, y);
        else
            waveformPath.lineTo(x, y);
    }
    graphics.setColour(juce::Colour(0x65ff9a35));
    graphics.strokePath(waveformPath, juce::PathStrokeType(2.0f));

    juce::Path waveformFill;
    waveformFill.startNewSubPath(display.getX() + 8.0f, midY);
    for (std::size_t i = 0; i < waveform.size(); ++i)
    {
        const auto x = display.getX() + 8.0f + static_cast<float>(i) * (display.getWidth() - 16.0f)
            / static_cast<float>(waveform.size() - 1);
        waveformFill.lineTo(x, midY - waveform[i] * amplitude);
    }
    waveformFill.lineTo(display.getRight() - 8.0f, midY);
    waveformFill.closeSubPath();
    graphics.setColour(juce::Colour(0x70ff9a35));
    graphics.fillPath(waveformFill);

    const auto leftLevel = std::clamp(audioProcessor.inputMeterLeft.load(), 0.0f, 1.0f);
    const auto rightLevel = std::clamp(audioProcessor.outputMeterRight.load(), 0.0f, 1.0f);
    graphics.setColour(juce::Colour(0xff6e777b));
    graphics.fillRoundedRectangle(852.0f, 175.0f, 12.0f, 160.0f, 6.0f);
    graphics.fillRoundedRectangle(1056.0f, 175.0f, 12.0f, 160.0f, 6.0f);
    graphics.setColour(orange());
    graphics.fillRoundedRectangle(852.0f, 335.0f - leftLevel * 150.0f, 12.0f, leftLevel * 150.0f, 6.0f);
    graphics.fillRoundedRectangle(1056.0f, 335.0f - rightLevel * 150.0f, 12.0f, rightLevel * 150.0f, 6.0f);

    graphics.setColour(juce::Colour(0xff303a40));
    graphics.setFont(juce::Font(13.0f, juce::Font::plain));
    graphics.drawText("AURALIS AUDIO  /  ALGORITHMIC STEREO REVERB", 38, 574, 650, 22, juce::Justification::centredLeft);
    graphics.setColour(juce::Colour(0xffe5e9ea));
    graphics.drawText("VST3", getWidth() - 92, 574, 48, 22, juce::Justification::centredRight);
}

void SpectraReverbAudioProcessorEditor::resized()
{
    presetBox.setBounds(842, 33, 236, 42);

    for (std::size_t i = 0; i < mainKnobs.size(); ++i)
    {
        const auto x = 30 + static_cast<int>(i) * 158;
        controlLabels[i]->setBounds(x, 130, 142, 30);
        mainKnobs[i]->setBounds(x + 2, 166, 138, 142);
        valueLabels[i]->setBounds(x, 316, 142, 30);
    }

    constexpr int detailWidth = 188;
    constexpr int detailGap = 24;
    constexpr int detailStartX = 46;
    for (std::size_t i = 0; i < detailSliders.size(); ++i)
    {
        const auto x = detailStartX + static_cast<int>(i) * (detailWidth + detailGap);
        const auto controlIndex = i + mainKnobs.size();
        controlLabels[controlIndex]->setBounds(x, 438, detailWidth, 26);
        detailSliders[i]->setBounds(x, 470, detailWidth, 34);
        valueLabels[controlIndex]->setBounds(x, 511, detailWidth, 26);
    }
}

juce::String SpectraReverbAudioProcessorEditor::formatValue(std::size_t index) const
{
    const auto value = controlFor(index)->getValue();
    switch (index)
    {
        case 0: return juce::String(value, 0) + " ms";
        case 1: return juce::String(value, 1) + " s";
        case 2:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8: return juce::String(juce::roundToInt(value * 100.0)) + "%";
        case 3: return juce::String(value, 2);
        case 9: return juce::String(value, 1) + " dB";
        default: return {};
    }
}

void SpectraReverbAudioProcessorEditor::updateMeterHistory()
{
    const auto input = std::max(audioProcessor.inputMeterLeft.load(), audioProcessor.inputMeterRight.load());
    const auto output = std::max(audioProcessor.outputMeterLeft.load(), audioProcessor.outputMeterRight.load());
    waveform[static_cast<std::size_t>(waveformPosition)] = std::clamp(std::max(input, output), 0.0f, 1.0f);
    waveformPosition = (waveformPosition + 1) % static_cast<int>(waveform.size());
    std::rotate(waveform.begin(), waveform.begin() + 1, waveform.end());
}

void SpectraReverbAudioProcessorEditor::timerCallback()
{
    for (std::size_t i = 0; i < valueLabels.size(); ++i)
        valueLabels[i]->setText(formatValue(i), juce::dontSendNotification);

    const auto presetId = audioProcessor.getCurrentProgram() + 1;
    if (presetBox.getSelectedId() != presetId)
        presetBox.setSelectedId(presetId, juce::dontSendNotification);

    updateMeterHistory();
    repaint();
}
