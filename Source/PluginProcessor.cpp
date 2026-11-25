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

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "decay", 1 }, "Decay",
        juce::NormalisableRange<float> (0.2f, 12.0f, 0.01f), 4.0f, " s"));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "damping", 1 }, "Damping",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.4f));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "width", 1 }, "Width",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 }, "Wet Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.25f));

    layout.add (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypass", 1 }, "Bypass", false));

    return layout;
}

bool R2RVERBAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();

    // This first stable release intentionally supports stereo only.
    return input == juce::AudioChannelSet::stereo()
        && output == juce::AudioChannelSet::stereo();
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
    const float size = parameters.getRawParameterValue ("size")->load();
    const float decay = parameters.getRawParameterValue ("decay")->load();
    const float damping = parameters.getRawParameterValue ("damping")->load();
    const float width = parameters.getRawParameterValue ("width")->load();

    const float modeSize = mode == 0 ? 0.12f : (mode == 1 ? -0.10f : 0.02f);
    const float decayAmount = juce::jlimit (0.0f, 1.0f, decay / 12.0f);

    juce::dsp::Reverb::Parameters p;
    p.roomSize = juce::jlimit (0.05f, 0.98f,
        0.18f + (0.58f * size) + (0.22f * decayAmount) + modeSize);
    p.damping = juce::jlimit (0.0f, 1.0f, damping);
    p.wetLevel = 1.0f;
    p.dryLevel = 0.0f;
    p.width = juce::jlimit (0.0f, 1.0f, width);
    p.freezeMode = 0.0f;
    reverb.setParameters (p);
}

void R2RVERBAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (2, buffer.getNumChannels());

    if (numChannels != 2 || parameters.getRawParameterValue ("bypass")->load() > 0.5f)
        return;

    const float wet = juce::jlimit (0.0f, 1.0f,
        parameters.getRawParameterValue ("mix")->load());
    const float dry = 1.0f - wet;

    updateReverbParameters();

    // Keep the dry signal in the host buffer and process a separate wet copy.
    wetBuffer.setSize (2, numSamples, false, false, true);
    wetBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
    wetBuffer.copyFrom (1, 0, buffer, 1, 0, numSamples);

    juce::dsp::AudioBlock<float> block (wetBuffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    reverb.process (context);

    for (int channel = 0; channel < 2; ++channel)
    {
        auto* dryData = buffer.getWritePointer (channel);
        const auto* wetData = wetBuffer.getReadPointer (channel);

        for (int sample = 0; sample < numSamples; ++sample)
            dryData[sample] = dryData[sample] * dry + wetData[sample] * wet;
    }
}

juce::AudioProcessorEditor* R2RVERBAudioProcessor::createEditor()
{
    return new R2RVERBAudioProcessorEditor (*this);
}

void R2RVERBAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    const auto state = parameters.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void R2RVERBAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;

    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new R2RVERBAudioProcessor();
}
