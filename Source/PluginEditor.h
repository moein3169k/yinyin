#pragma once
#include <JuceHeader.h>

class YingyangAudioProcessor final : public juce::AudioProcessor
{
public:
    YingyangAudioProcessor();
    ~YingyangAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "YINGYANG"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;

private:
    static APVTS::ParameterLayout createParameterLayout();
    float get (const char* id) const;
    static float dbToGain (float db) noexcept;
    static float softClip (float x, float drive) noexcept;

    double sr = 44100.0;
    
    // Maksimum kanal sayısını güvenli olması için 8'e çıkardık, FL Studio kısıtlamalarından kurtulduk.
    float lowState[8] { 0.0f };
    float highState[8] { 0.0f };
    float deEssLow[8] { 0.0f };
    float fetEnv[8] { 0.0f };
    float optoEnv[8] { 0.0f };
    float noiseEnv[8] { 0.0f };
    float deEssEnv[8] { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YingyangAudioProcessor)
};
