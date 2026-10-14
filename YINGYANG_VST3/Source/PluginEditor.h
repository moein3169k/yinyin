#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class YingyangAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit YingyangAudioProcessorEditor (YingyangAudioProcessor&);
    ~YingyangAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    struct Knob : juce::Component
    {
        juce::Slider slider;
        juce::Label label;
        Knob (YingyangAudioProcessor& p, const juce::String& id, const juce::String& title);
        void resized() override;
    };
    void drawYinYang (juce::Graphics&, juce::Rectangle<float> bounds);
    YingyangAudioProcessor& processor;
    juce::Label title, subtitle, toneHeading, compHeading, cleanHeading, outputHeading;
    std::unique_ptr<Knob> bass, mid, treble, sat, blend, thresh, ratio, attack, release, deess, noise, output;
    std::vector<std::unique_ptr<Knob>> knobs;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YingyangAudioProcessorEditor)
};
