#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DMFEchoAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DMFEchoAudioProcessorEditor (DMFEchoAudioProcessor&);
    ~DMFEchoAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    class NeonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    } look;
    struct Knob { std::unique_ptr<juce::Slider> slider; std::unique_ptr<juce::Label> label, value; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment; };
    struct Menu { std::unique_ptr<juce::ComboBox> box; std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment; };
    void timerCallback() override;
    void drawVisualizer (juce::Graphics&, juce::Rectangle<float>);
    DMFEchoAudioProcessor& processor;
    juce::ToggleButton sync {"BPM SYNC"}, chordMode {"CHORD MODE"};
    juce::ComboBox division, preset;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment, chordAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionAttachment;
    Menu key, scale, motion, chord;
    Knob knobs[6];
    juce::TextButton saveButton {"▣"}, settingsButton {"LOAD"};
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Label logo, brand, title, leftTime, footerBrand;
    float visualPhase = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DMFEchoAudioProcessorEditor)
};
