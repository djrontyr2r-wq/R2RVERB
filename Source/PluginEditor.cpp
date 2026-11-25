#include "PluginEditor.h"

namespace
{
    const juce::Colour black (0xff090a0d);
    const juce::Colour panel (0xff15171c);
    const juce::Colour gold (0xffd6ad55);
    const juce::Colour paleGold (0xffffdf91);
    const juce::Colour muted (0xff8c8b86);
}

GoldLookAndFeel::GoldLookAndFeel()
{
    setColour (juce::Slider::thumbColourId, paleGold);
    setColour (juce::Slider::rotarySliderFillColourId, gold);
    setColour (juce::Slider::rotarySliderOutlineColourId, 0xff39352b);
    setColour (juce::Slider::textBoxTextColourId, paleGold);
    setColour (juce::Slider::textBoxBackgroundColourId, panel);
    setColour (juce::Slider::textBoxOutlineColourId, 0xff51452d);
    setColour (juce::ComboBox::backgroundColourId, panel);
    setColour (juce::ComboBox::textColourId, paleGold);
    setColour (juce::ComboBox::outlineColourId, gold);
    setColour (juce::Label::textColourId, paleGold);
    setColour (juce::ToggleButton::textColourId, paleGold);
}

void GoldLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                        float pos, float start, float end, juce::Slider&)
{
    const float diameter = juce::jmin (w, h) * 0.68f;
    const float cx = x + w * 0.5f;
    const float cy = y + h * 0.43f;
    const float radius = diameter * 0.5f;
    const float angle = start + pos * (end - start);

    g.setColour (juce::Colour (0xff25272c));
    g.fillEllipse (cx - radius, cy - radius, diameter, diameter);
    g.setColour (juce::Colour (0xff050608));
    g.drawEllipse (cx - radius, cy - radius, diameter, diameter, 2.0f);
    g.setColour (gold);
    g.drawEllipse (cx - radius + 3, cy - radius + 3, diameter - 6, diameter - 6, 1.4f);

    juce::Path arc;
    arc.addCentredArc (cx, cy, radius - 5, radius - 5, 0.0f, start, angle, true);
    g.setColour (gold);
    g.strokePath (arc, juce::PathStrokeType (3.0f));

    juce::Path pointer;
    pointer.addRoundedRectangle (-2.0f, -radius + 9.0f, 4.0f, radius * 0.48f, 2.0f);
    g.setColour (paleGold);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (cx, cy));
}

R2RVERBAudioProcessorEditor::R2RVERBAudioProcessorEditor (R2RVERBAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&goldLook);
    setSize (760, 520);

    brandLabel.setText ("DJ R2R MUSIC", juce::dontSendNotification);
    brandLabel.setFont (juce::Font (juce::FontOptions (23.0f, juce::Font::bold)));
    brandLabel.setColour (juce::Label::textColourId, paleGold);
    addAndMakeVisible (brandLabel);

    subtitleLabel.setText ("ADVANCED REVERB  /  HALL  •  ROOM  •  PLATE", juce::dontSendNotification);
    subtitleLabel.setFont (juce::Font (juce::FontOptions (11.0f)));
    subtitleLabel.setColour (juce::Label::textColourId, muted);
    addAndMakeVisible (subtitleLabel);

    modeLabel.setText ("ALGORITHM", juce::dontSendNotification);
    modeLabel.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    addAndMakeVisible (modeLabel);
    modeBox.addItem ("HALL", 1);
    modeBox.addItem ("ROOM", 2);
    modeBox.addItem ("PLATE", 3);
    addAndMakeVisible (modeBox);
    modeAttachment = std::make_unique<Attachment::ComboBoxAttachment> (p.parameters, "mode", modeBox);

    const std::array<std::pair<juce::Slider*, juce::String>, 8> controls {{
        { &size, "size" }, { &decay, "decay" }, { &predelay, "predelay" },
        { &damping, "damping" }, { &width, "width" }, { &mix, "mix" },
        { &lowcut, "lowcut" }, { &highcut, "highcut" }
    }};
    const std::array<juce::String, 8> names {{
        "SIZE", "DECAY", "PRE-DELAY", "DAMPING",
        "WIDTH", "WET MIX", "LOW CUT", "HIGH CUT"
    }};
    for (size_t i = 0; i < controls.size(); ++i)
    {
        configureSlider (*controls[i].first, names[i]);
        sliderAttachments.push_back (std::make_unique<Attachment::SliderAttachment> (
            p.parameters, controls[i].second, *controls[i].first));
    }

    addAndMakeVisible (freeze);
    addAndMakeVisible (bypass);
    freezeAttachment = std::make_unique<Attachment::ButtonAttachment> (p.parameters, "freeze", freeze);
    bypassAttachment = std::make_unique<Attachment::ButtonAttachment> (p.parameters, "bypass", bypass);
}

R2RVERBAudioProcessorEditor::~R2RVERBAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void R2RVERBAudioProcessorEditor::configureSlider (juce::Slider& slider, const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 20);
    slider.setName (labelText);
    addAndMakeVisible (slider);
    auto label = std::make_unique<juce::Label>();
    label->setText (labelText, juce::dontSendNotification);
    label->setJustificationType (juce::Justification::centred);
    label->setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    label->setColour (juce::Label::textColourId, muted);
    addAndMakeVisible (*label);
    labels.push_back (std::move (label));
}

void R2RVERBAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (black);
    g.setColour (gold);
    g.drawRoundedRectangle (10.0f, 10.0f, (float) getWidth() - 20.0f,
                            (float) getHeight() - 20.0f, 12.0f, 1.2f);
    g.setColour (panel);
    g.fillRoundedRectangle (20.0f, 20.0f, (float) getWidth() - 40.0f, 86.0f, 9.0f);
    g.setColour (gold);
    g.drawLine (30.0f, 111.0f, (float) getWidth() - 30.0f, 111.0f, 1.0f);
    g.setColour (muted);
    g.setFont (11.0f);
    g.drawText ("R2RVERB  •  STUDIO SPACE DESIGN", 28, getHeight() - 28,
                getWidth() - 56, 14, juce::Justification::centredRight);
}

void R2RVERBAudioProcessorEditor::resized()
{
    brandLabel.setBounds (34, 28, 250, 32);
    subtitleLabel.setBounds (36, 63, 350, 20);
    modeLabel.setBounds (480, 31, 100, 18);
    modeBox.setBounds (480, 54, 230, 30);

    const int left = 28;
    const int top = 130;
    const int cellW = 176;
    const int cellH = 164;
    std::array<juce::Slider*, 8> sliders {{
        &size, &decay, &predelay, &damping, &width, &mix, &lowcut, &highcut
    }};
    for (int i = 0; i < (int) sliders.size(); ++i)
    {
        const int col = i % 4;
        const int row = i / 4;
        const int x = left + col * cellW;
        const int y = top + row * cellH;
        sliders[(size_t) i]->setBounds (x, y + 18, 150, 126);
        labels[(size_t) i]->setBounds (x, y, 150, 18);
    }
    freeze.setBounds (34, 466, 110, 26);
    bypass.setBounds (160, 466, 110, 26);
}
