#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

class R2RVERBAudioProcessor final : public juce::AudioProcessor
{
public:
    R2RVERBAudioProcessor();
    ~R2RVERBAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "R2RVERB"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 20.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;
    static APVTS::ParameterLayout createParameterLayout();

private:
    juce::dsp::Reverb reverb;
    juce::dsp::ProcessSpec spec {};
    juce::AudioBuffer<float> wetBuffer;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelay { 96000 };
    std::array<juce::dsp::IIR::Filter<float>, 2> lowCutFilters;
    std::array<juce::dsp::IIR::Filter<float>, 2> highCutFilters;
    float smoothedWet = 0.25f;
    float smoothedDry = 0.75f;

    void updateReverbParameters();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2RVERBAudioProcessor)
};
