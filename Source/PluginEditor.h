#pragma once
#include "PluginProcessor.h"

class GoldLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    GoldLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;
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
    R2RVERBAudioProcessor& audioProcessor;
    GoldLookAndFeel goldLook;
    juce::ComboBox modeBox;
    juce::Label modeLabel, brandLabel, subtitleLabel;
    juce::Slider size, decay, predelay, damping, width, mix, lowcut, highcut;
    juce::ToggleButton freeze { "FREEZE" }, bypass { "BYPASS" };
    std::vector<std::unique_ptr<juce::Label>> labels;
    std::vector<std::unique_ptr<Attachment::SliderAttachment>> sliderAttachments;
    std::unique_ptr<Attachment::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<Attachment::ButtonAttachment> freezeAttachment, bypassAttachment;

    void configureSlider (juce::Slider&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2RVERBAudioProcessorEditor)
};
