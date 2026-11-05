#pragma once
#include <JuceHeader.h>
#include "DSP.h"

class DMFEchoAudioProcessor : public juce::AudioProcessor
{
public:
    DMFEchoAudioProcessor();
    ~DMFEchoAudioProcessor() override = default;
    void prepareToPlay (double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "DMF ECHO"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }
    int getNumPrograms() override { return 8; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    void applyFactoryPreset (int index);
    bool savePresetToFile (const juce::File& file) const;
    bool loadPresetFromFile (const juce::File& file);
    float getMeter() const noexcept { return meter.load(); }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
    dmf::DelayEngine engine;
    std::atomic<float> meter { 0.0f };
    int currentProgram = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DMFEchoAudioProcessor)
};
