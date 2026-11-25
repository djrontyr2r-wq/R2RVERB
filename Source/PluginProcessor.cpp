#include "PluginProcessor.h"
#include "PluginEditor.h"

R2RVERBAudioProcessor::R2RVERBAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "R2RVERB_STATE", createParameterLayout())
{
}

R2RVERBAudioProcessor::APVTS::ParameterLayout R2RVERBAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "mode", 1 }, "Reverb Mode",
        juce::StringArray { "HALL", "ROOM", "PLATE" }, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "size", 1 }, "Size",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.72f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "decay", 1 }, "Decay",
        juce::NormalisableRange<float> (0.2f, 12.0f, 0.01f, 0.5f), 3.8f, " s"));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "predelay", 1 }, "Pre-Delay",
        juce::NormalisableRange<float> (0.0f, 250.0f, 0.1f), 24.0f, " ms"));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "damping", 1 }, "Damping",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.38f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "width", 1 }, "Width",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 }, "Wet Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.25f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "lowcut", 1 }, "Low Cut",
        juce::NormalisableRange<float> (20.0f, 1000.0f, 1.0f, 0.35f), 80.0f, " Hz"));
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "highcut", 1 }, "High Cut",
        juce::NormalisableRange<float> (2000.0f, 20000.0f, 1.0f, 0.35f), 16000.0f, " Hz"));
    layout.add (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "freeze", 1 }, "Freeze", false));
    layout.add (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "bypass", 1 }, "Bypass", false));
    return layout;
}

void R2RVERBAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint> (getTotalNumOutputChannels());

    reverb.reset();
    reverb.prepare (spec);

    wetBuffer.setSize (static_cast<int> (spec.numChannels), samplesPerBlock);

    preDelay.prepare (spec);
    preDelay.reset();

    for (auto& filter : lowCutFilters)
        filter.prepare (spec);

    for (auto& filter : highCutFilters)
        filter.prepare (spec);

    updateReverbParameters();

    smoothedWet = parameters.getRawParameterValue ("mix")->load();
    smoothedDry = 1.0f - smoothedWet;
}

void R2RVERBAudioProcessor::releaseResources()
{
    reverb.reset();
    preDelay.reset();
    for (auto& filter : lowCutFilters) filter.reset();
    for (auto& filter : highCutFilters) filter.reset();
    wetBuffer.setSize (0, 0);
}

bool R2RVERBAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();
    return mainIn == mainOut && (mainOut == juce::AudioChannelSet::mono()
                              || mainOut == juce::AudioChannelSet::stereo());
}

void R2RVERBAudioProcessor::updateReverbParameters()
{
    const auto mode = static_cast<int> (parameters.getRawParameterValue ("mode")->load());
    const auto size = parameters.getRawParameterValue ("size")->load();
    const auto decay = parameters.getRawParameterValue ("decay")->load();
    const auto damping = parameters.getRawParameterValue ("damping")->load();
    const auto width = parameters.getRawParameterValue ("width")->load();
    const auto freeze = parameters.getRawParameterValue ("freeze")->load() > 0.5f;

    // Three tuned voicings using JUCE's stereo reverb network.
    const float modeSize  = mode == 0 ? 0.88f : (mode == 1 ? 0.38f : 0.62f);
    const float modeDamp  = mode == 0 ? 0.30f : (mode == 1 ? 0.62f : 0.22f);
    const float modeWidth = mode == 1 ? 0.72f : 1.0f;

    juce::dsp::Reverb::Parameters p;
    const float decayAmount = juce::jlimit (0.0f, 1.0f, decay / 12.0f);
    p.roomSize = juce::jlimit (0.0f, 1.0f,
        0.12f + 0.58f * size + 0.30f * decayAmount * modeSize);
    p.damping = juce::jlimit (0.0f, 1.0f, 0.5f * modeDamp + 0.5f * damping);
    p.wetLevel = 1.0f;
    p.dryLevel = 0.0f;
    p.width = juce::jlimit (0.0f, 1.0f, width * modeWidth);
    p.freezeMode = freeze ? 1.0f : 0.0f;
    reverb.setParameters (p);

    // Decay and size jointly shape the reverb network's decay character.
}

void R2RVERBAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumOutputChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < channels; ++ch)
        buffer.clear (ch, 0, samples);

    if (parameters.getRawParameterValue ("bypass")->load() > 0.5f)
        return;

    updateReverbParameters();
    const float targetWet = parameters.getRawParameterValue ("mix")->load();
    const float targetDry = 1.0f - targetWet;
    const float preDelayMs = parameters.getRawParameterValue ("predelay")->load();
    const float lowCutHz = parameters.getRawParameterValue ("lowcut")->load();
    const float highCutHz = parameters.getRawParameterValue ("highcut")->load();
    preDelay.setDelay (preDelayMs * static_cast<float> (spec.sampleRate) / 1000.0f);
    const auto safeLowCut = juce::jlimit (20.0f, static_cast<float> (spec.sampleRate * 0.45), lowCutHz);
    const auto safeHighCut = juce::jlimit (2000.0f, static_cast<float> (spec.sampleRate * 0.45), highCutHz);
    auto hp = juce::dsp::IIR::Coefficients<float>::makeHighPass (spec.sampleRate, safeLowCut);
    auto lp = juce::dsp::IIR::Coefficients<float>::makeLowPass (spec.sampleRate, safeHighCut);
    for (int ch = 0; ch < channels; ++ch)
    {
        lowCutFilters[(size_t) ch].coefficients = hp;
        highCutFilters[(size_t) ch].coefficients = lp;
    }

    wetBuffer.setSize (channels, samples, false, false, true);
    for (int ch = 0; ch < channels; ++ch)
    {
        for (int i = 0; i < samples; ++i)
        {
            preDelay.pushSample (ch, buffer.getSample (ch, i));
            auto sample = preDelay.popSample (ch);
            sample = lowCutFilters[(size_t) ch].processSample (sample);
            sample = highCutFilters[(size_t) ch].processSample (sample);
            wetBuffer.setSample (ch, i, sample);
        }
    }

    juce::dsp::AudioBlock<float> reverbBlock (wetBuffer);
    juce::dsp::ProcessContextReplacing<float> reverbContext (reverbBlock);
    reverb.process (reverbContext);

    for (int i = 0; i < samples; ++i)
    {
        smoothedWet += 0.002f * (targetWet - smoothedWet);
        smoothedDry += 0.002f * (targetDry - smoothedDry);
        for (int ch = 0; ch < channels; ++ch)
            buffer.setSample (ch, i, buffer.getSample (ch, i) * smoothedDry
                                   + wetBuffer.getSample (ch, i) * smoothedWet);
    }
}

juce::AudioProcessorEditor* R2RVERBAudioProcessor::createEditor()
{
    return new R2RVERBAudioProcessorEditor (*this);
}

void R2RVERBAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void R2RVERBAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new R2RVERBAudioProcessor();
}
