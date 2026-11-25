#include "PluginProcessor.h"
#include "PluginEditor.h"

R2RVERBAudioProcessor::R2RVERBAudioProcessor()
    : juce::AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "R2RVERB_STATE", createParameterLayout())
{
}

R2RVERBAudioProcessor::APVTS::ParameterLayout
R2RVERBAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "mode", 1 }, "Mode",
        juce::StringArray { "HALL", "ROOM", "PLATE" }, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "size", 1 }, "Size",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.68f));
    layout.add (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "decayDivision", 1 }, "Decay Time",
        juce::StringArray { "1/2", "1/3", "1/4", "1/6", "1/8", "1/16", "1/32" }, 2));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "damping", 1 }, "Damping",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.4f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "width", 1 }, "Width",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 }, "Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.25f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "dry", 1 }, "Dry",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    layout.add (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypass", 1 }, "Bypass", false));
    return layout;
}

bool R2RVERBAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void R2RVERBAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    processSpec.sampleRate = sampleRate;
    processSpec.maximumBlockSize = static_cast<juce::uint32> (juce::jmax (1, samplesPerBlock));
    processSpec.numChannels = 2;
    reverb.reset();
    reverb.prepare (processSpec);
    wetBuffer.setSize (2, juce::jmax (1, samplesPerBlock), false, true, true);
    updateReverbParameters();
}

void R2RVERBAudioProcessor::releaseResources()
{
    reverb.reset();
    wetBuffer.setSize (0, 0);
}

void R2RVERBAudioProcessor::updateReverbParameters()
{
    const int mode = juce::roundToInt (parameters.getRawParameterValue ("mode")->load());
    const int division = juce::roundToInt (parameters.getRawParameterValue ("decayDivision")->load());
    const float size = parameters.getRawParameterValue ("size")->load();
    const float damping = parameters.getRawParameterValue ("damping")->load();
    const float width = parameters.getRawParameterValue ("width")->load();

    // Musical divisions expressed as beats: half, triplet, quarter, etc.
    constexpr std::array<double, 7> beats { 2.0, 4.0 / 3.0, 1.0, 2.0 / 3.0, 0.5, 0.25, 0.125 };
    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
            if (auto hostBpm = position->getBpm())
                bpm = juce::jlimit (20.0, 300.0, *hostBpm);
    }

    const double decaySeconds = (60.0 / bpm) * beats[(size_t) juce::jlimit (0, 6, division)];
    const float decayAmount = (float) juce::jlimit (0.0, 1.0, decaySeconds / 4.0);
    const float modeOffset = mode == 0 ? 0.10f : (mode == 1 ? -0.10f : 0.02f);

    juce::dsp::Reverb::Parameters p;
    p.roomSize = juce::jlimit (0.05f, 0.98f,
        0.12f + (0.30f * size) + (0.55f * decayAmount) + modeOffset);
    p.damping = juce::jlimit (0.0f, 1.0f, damping);
    p.wetLevel = 1.0f;
    p.dryLevel = 0.0f;
    p.width = juce::jlimit (0.0f, 1.0f, width);
    p.freezeMode = 0.0f;
    reverb.setParameters (p);
}

void R2RVERBAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    if (buffer.getNumChannels() != 2 || parameters.getRawParameterValue ("bypass")->load() > 0.5f)
        return;

    const float wet = juce::jlimit (0.0f, 1.0f, parameters.getRawParameterValue ("mix")->load());
    const float dry = juce::jlimit (0.0f, 1.0f, parameters.getRawParameterValue ("dry")->load());

    updateReverbParameters();
    wetBuffer.setSize (2, numSamples, false, false, false);
    wetBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
    wetBuffer.copyFrom (1, 0, buffer, 1, 0, numSamples);

    juce::dsp::AudioBlock<float> block (wetBuffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    reverb.process (context);

    for (int channel = 0; channel < 2; ++channel)
    {
        auto* output = buffer.getWritePointer (channel);
        const auto* wetData = wetBuffer.getReadPointer (channel);
        for (int sample = 0; sample < numSamples; ++sample)
            output[sample] = output[sample] * dry + wetData[sample] * wet;
    }
}

juce::AudioProcessorEditor* R2RVERBAudioProcessor::createEditor()
{
    return new R2RVERBAudioProcessorEditor (*this);
}

void R2RVERBAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void R2RVERBAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0) return;
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new R2RVERBAudioProcessor();
}
