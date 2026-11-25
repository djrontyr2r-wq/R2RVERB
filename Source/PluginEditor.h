#pragma once
#include "PluginProcessor.h"

class R2RLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    R2RLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float,
                           float, float, juce::Slider&) override;
};

class R2RVERBAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit R2RVERBAudioProcessorEditor (R2RVERBAudioProcessor&);
    ~R2RVERBAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using Attachment = juce::AudioProcessorValueTreeState;

    R2RVERBAudioProcessor& processor;
    R2RLookAndFeel lookAndFeel;
    juce::Label titleLabel, subtitleLabel, modeLabel;
    juce::ComboBox modeBox;
    std::array<juce::Slider, 5> knobs;
    std::array<juce::Label, 5> knobLabels;
    juce::ToggleButton bypassButton { "BYPASS" };
    std::vector<std::unique_ptr<Attachment::SliderAttachment>> sliderAttachments;
    std::unique_ptr<Attachment::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<Attachment::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2RVERBAudioProcessorEditor)
};
