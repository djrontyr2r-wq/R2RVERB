#include "PluginEditor.h"

namespace
{
    const juce::Colour background (0xfff5f8fc);
    const juce::Colour panel (0xffffffff);
    const juce::Colour cyan (0xff00a99a);
    const juce::Colour blue (0xff477dff);
    const juce::Colour ink (0xff172638);
    const juce::Colour muted (0xff607286);
    const juce::Colour track (0xffdce5ee);
}

R2RLookAndFeel::R2RLookAndFeel()
{
    setColour (juce::Slider::thumbColourId, juce::Colours::white);
    setColour (juce::Slider::rotarySliderFillColourId, cyan);
    setColour (juce::Slider::rotarySliderOutlineColourId, track);
    setColour (juce::Slider::textBoxTextColourId, ink);
    setColour (juce::Slider::textBoxBackgroundColourId, panel);
    setColour (juce::Slider::textBoxOutlineColourId, track);
    setColour (juce::ComboBox::backgroundColourId, panel);
    setColour (juce::ComboBox::textColourId, ink);
    setColour (juce::ComboBox::outlineColourId, cyan);
    setColour (juce::Label::textColourId, ink);
    setColour (juce::ToggleButton::textColourId, cyan);
}

void R2RLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                      float position, float start, float end, juce::Slider&)
{
    const float diameter = juce::jmin (w, h) * 0.66f;
    const float cx = x + w * 0.5f, cy = y + h * 0.40f, radius = diameter * 0.5f;
    const float angle = start + position * (end - start);
    g.setColour (panel);
    g.fillEllipse (cx - radius, cy - radius, diameter, diameter);
    g.setColour (track);
    g.drawEllipse (cx - radius, cy - radius, diameter, diameter, 2.0f);

    juce::Path trackArc;
    trackArc.addCentredArc (cx, cy, radius - 5.0f, radius - 5.0f, 0.0f, start, end, true);
    g.setColour (track);
    g.strokePath (trackArc, juce::PathStrokeType (4.0f));

    juce::Path valueArc;
    valueArc.addCentredArc (cx, cy, radius - 5.0f, radius - 5.0f, 0.0f, start, angle, true);
    g.setColour (cyan);
    g.strokePath (valueArc, juce::PathStrokeType (4.0f));

    juce::Path pointer;
    pointer.addRoundedRectangle (-2.0f, -radius + 9.0f, 4.0f, radius * 0.42f, 2.0f);
    g.setColour (ink);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (cx, cy));
}

R2RVERBAudioProcessorEditor::R2RVERBAudioProcessorEditor (R2RVERBAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (800, 420);

    titleLabel.setText ("DJ R2R  /  R2RVERB 2.0", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, cyan);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("STUDIO SPACE  •  TEMPO-SYNCED REVERB", juce::dontSendNotification);
    subtitleLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    subtitleLabel.setColour (juce::Label::textColourId, muted);
    addAndMakeVisible (subtitleLabel);

    modeLabel.setText ("REVERB TYPE", juce::dontSendNotification);
    modeLabel.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    modeLabel.setColour (juce::Label::textColourId, muted);
    addAndMakeVisible (modeLabel);
    modeBox.addItem ("HALL", 1);
    modeBox.addItem ("ROOM", 2);
    modeBox.addItem ("PLATE", 3);
    addAndMakeVisible (modeBox);
    modeAttachment = std::make_unique<Attachment::ComboBoxAttachment> (processor.parameters, "mode", modeBox);

    decayLabel.setText ("DECAY TIME", juce::dontSendNotification);
    decayLabel.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    decayLabel.setColour (juce::Label::textColourId, muted);
    addAndMakeVisible (decayLabel);
    const juce::StringArray divisions { "1/2", "1/3", "1/4", "1/6", "1/8", "1/16", "1/32" };
    for (int i = 0; i < divisions.size(); ++i) decayBox.addItem (divisions[i], i + 1);
    addAndMakeVisible (decayBox);
    decayAttachment = std::make_unique<Attachment::ComboBoxAttachment> (processor.parameters, "decayDivision", decayBox);

    const std::array<const char*, 6> ids { "size", "damping", "width", "dry", "mix", "dry" };
    const std::array<const char*, 6> names { "SIZE", "DAMPING", "WIDTH", "DRY", "WET", " " };
    // Five useful rotary controls plus a sixth DECAY character is handled by the dropdown above.
    for (size_t i = 0; i < 5; ++i)
    {
        auto& slider = knobs[i];
        slider.setLookAndFeel (&lookAndFeel);
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 20);
        addAndMakeVisible (slider);
        knobLabels[i].setText (names[i], juce::dontSendNotification);
        knobLabels[i].setJustificationType (juce::Justification::centred);
        knobLabels[i].setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
        knobLabels[i].setColour (juce::Label::textColourId, muted);
        addAndMakeVisible (knobLabels[i]);
        sliderAttachments.push_back (std::make_unique<Attachment::SliderAttachment> (processor.parameters, ids[i], slider));
    }

    addAndMakeVisible (bypassButton);
    bypassAttachment = std::make_unique<Attachment::ButtonAttachment> (processor.parameters, "bypass", bypassButton);
}

R2RVERBAudioProcessorEditor::~R2RVERBAudioProcessorEditor()
{
    for (auto& knob : knobs) knob.setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void R2RVERBAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (background);
    g.setColour (panel);
    g.fillRoundedRectangle (14.0f, 14.0f, (float) getWidth() - 28.0f, 88.0f, 12.0f);
    g.setColour (track);
    g.drawRoundedRectangle (8.0f, 8.0f, (float) getWidth() - 16.0f, (float) getHeight() - 16.0f, 14.0f, 1.2f);
    g.setColour (cyan);
    g.drawLine (24.0f, 111.0f, (float) getWidth() - 24.0f, 111.0f, 1.5f);
    g.setColour (muted);
    g.setFont (11.0f);
    g.drawText ("NATIVE STEREO VST3  •  DJ R2R MUSIC", 24, getHeight() - 27, getWidth() - 48, 14, juce::Justification::centredRight);
}

void R2RVERBAudioProcessorEditor::resized()
{
    titleLabel.setBounds (30, 27, 360, 34);
    subtitleLabel.setBounds (32, 64, 360, 20);
    modeLabel.setBounds (430, 25, 130, 16);
    modeBox.setBounds (430, 48, 150, 30);
    decayLabel.setBounds (600, 25, 150, 16);
    decayBox.setBounds (600, 48, 150, 30);

    const int left = 22, top = 132, cellWidth = 148;
    for (int i = 0; i < 5; ++i)
    {
        const int x = left + i * cellWidth;
        knobLabels[(size_t) i].setBounds (x, top, 136, 18);
        knobs[(size_t) i].setBounds (x, top + 18, 136, 158);
    }
    bypassButton.setBounds (30, 340, 120, 26);
}
