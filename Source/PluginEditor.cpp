#include "PluginEditor.h"

namespace
{
    const juce::Colour background (0xff080b12);
    const juce::Colour panel (0xff111a28);
    const juce::Colour cyan (0xff28e0d0);
    const juce::Colour blue (0xff477dff);
    const juce::Colour white (0xffe8f4ff);
    const juce::Colour muted (0xff8293a8);
}

R2RLookAndFeel::R2RLookAndFeel()
{
    setColour (juce::Slider::thumbColourId, white);
    setColour (juce::Slider::rotarySliderFillColourId, cyan);
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff26394b));
    setColour (juce::Slider::textBoxTextColourId, white);
    setColour (juce::Slider::textBoxBackgroundColourId, panel);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff26394b));
    setColour (juce::ComboBox::backgroundColourId, panel);
    setColour (juce::ComboBox::textColourId, white);
    setColour (juce::ComboBox::outlineColourId, cyan);
    setColour (juce::Label::textColourId, white);
    setColour (juce::ToggleButton::textColourId, cyan);
}

void R2RLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                      float position, float start, float end,
                                      juce::Slider&)
{
    const float diameter = juce::jmin (w, h) * 0.66f;
    const float cx = x + w * 0.5f;
    const float cy = y + h * 0.40f;
    const float radius = diameter * 0.5f;
    const float angle = start + position * (end - start);

    g.setColour (juce::Colour (0xff182434));
    g.fillEllipse (cx - radius, cy - radius, diameter, diameter);
    g.setColour (juce::Colour (0xff30445a));
    g.drawEllipse (cx - radius, cy - radius, diameter, diameter, 2.0f);

    juce::Path track;
    track.addCentredArc (cx, cy, radius - 5.0f, radius - 5.0f, 0.0f, start, end, true);
    g.setColour (juce::Colour (0xff26394b));
    g.strokePath (track, juce::PathStrokeType (4.0f));

    juce::Path valueArc;
    valueArc.addCentredArc (cx, cy, radius - 5.0f, radius - 5.0f, 0.0f, start, angle, true);
    g.setColour (cyan);
    g.strokePath (valueArc, juce::PathStrokeType (4.0f));

    juce::Path pointer;
    pointer.addRoundedRectangle (-2.0f, -radius + 9.0f, 4.0f, radius * 0.42f, 2.0f);
    g.setColour (white);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (cx, cy));
}

R2RVERBAudioProcessorEditor::R2RVERBAudioProcessorEditor (R2RVERBAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (680, 390);

    titleLabel.setText ("DJ R2R  /  R2RVERB 2.0", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, cyan);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("STUDIO SPACE  •  HALL / ROOM / PLATE", juce::dontSendNotification);
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
    modeAttachment = std::make_unique<Attachment::ComboBoxAttachment> (
        processor.parameters, "mode", modeBox);

    const std::array<const char*, 5> ids { "size", "decay", "damping", "width", "mix" };
    const std::array<const char*, 5> names { "SIZE", "DECAY", "DAMPING", "WIDTH", "WET MIX" };

    for (size_t i = 0; i < knobs.size(); ++i)
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

        sliderAttachments.push_back (std::make_unique<Attachment::SliderAttachment> (
            processor.parameters, ids[i], slider));
    }

    addAndMakeVisible (bypassButton);
    bypassAttachment = std::make_unique<Attachment::ButtonAttachment> (
        processor.parameters, "bypass", bypassButton);
}

R2RVERBAudioProcessorEditor::~R2RVERBAudioProcessorEditor()
{
    for (auto& knob : knobs)
        knob.setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void R2RVERBAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (background);

    g.setColour (panel);
    g.fillRoundedRectangle (14.0f, 14.0f, (float) getWidth() - 28.0f, 88.0f, 12.0f);

    g.setColour (cyan);
    g.drawRoundedRectangle (8.0f, 8.0f, (float) getWidth() - 16.0f,
                            (float) getHeight() - 16.0f, 14.0f, 1.2f);
    g.setColour (blue);
    g.drawLine (24.0f, 111.0f, (float) getWidth() - 24.0f, 111.0f, 1.0f);

    g.setColour (muted);
    g.setFont (11.0f);
    g.drawText ("NATIVE STEREO VST3  •  DJ R2R MUSIC", 24, getHeight() - 27,
                getWidth() - 48, 14, juce::Justification::centredRight);
}

void R2RVERBAudioProcessorEditor::resized()
{
    titleLabel.setBounds (30, 27, 360, 34);
    subtitleLabel.setBounds (32, 64, 360, 20);
    modeLabel.setBounds (455, 28, 150, 16);
    modeBox.setBounds (455, 51, 190, 30);

    const int left = 24;
    const int top = 132;
    const int cellWidth = 126;
    for (int i = 0; i < (int) knobs.size(); ++i)
    {
        const int x = left + i * cellWidth;
        knobLabels[(size_t) i].setBounds (x, top, 116, 18);
        knobs[(size_t) i].setBounds (x, top + 18, 116, 142);
    }

    bypassButton.setBounds (30, 320, 120, 26);
}
