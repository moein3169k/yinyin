#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::String paramID (const char* s) { return juce::String (s); }
    float smoothCoeff (float ms, double sampleRate)
    {
        const float seconds = juce::jmax (0.0001f, ms * 0.001f);
        return std::exp (-1.0f / (seconds * static_cast<float> (sampleRate)));
    }
}

YingyangAudioProcessor::YingyangAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "YINGYANG_STATE", createParameterLayout())
{}

YingyangAudioProcessor::APVTS::ParameterLayout YingyangAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout p;
    auto addFloat = [&p] (const char* id, const char* name, float min, float max, float def, float step = 0.01f, const char* unit = "")
    {
        p.add (std::make_unique<juce::AudioParameterFloat> (paramID (id), name,
            juce::NormalisableRange<float> (min, max, step), def, unit));
    };
    addFloat ("BASS", "Bass", -12.0f, 12.0f, 0.0f, 0.1f, " dB");
    addFloat ("MID", "Mid", -12.0f, 12.0f, 0.0f, 0.1f, " dB");
    addFloat ("TREBLE", "Treble", -12.0f, 12.0f, 0.0f, 0.1f, " dB");
    addFloat ("SAT", "Saturation", 0.0f, 100.0f, 18.0f, 0.1f, "%");
    addFloat ("BLEND", "FET / Opto", 0.0f, 100.0f, 55.0f, 0.1f, "%");
    addFloat ("THRESH", "Threshold", -36.0f, 0.0f, -18.0f, 0.1f, " dB");
    addFloat ("RATIO", "Ratio", 1.0f, 20.0f, 4.0f, 0.1f, ":1");
    addFloat ("ATTACK", "Attack", 0.1f, 100.0f, 8.0f, 0.1f, " ms");
    addFloat ("RELEASE", "Release", 10.0f, 1000.0f, 100.0f, 1.0f, " ms");
    addFloat ("DEESS", "De-Esser", 0.0f, 100.0f, 25.0f, 0.1f, "%");
    addFloat ("NOISE", "Noise Reduction", 0.0f, 100.0f, 0.0f, 0.1f, "%");
    addFloat ("OUTPUT", "Output", -18.0f, 18.0f, 0.0f, 0.1f, " dB");
    return p;
}

float YingyangAudioProcessor::get (const char* id) const
{
    if (auto* v = parameters.getRawParameterValue (id)) return v->load();
    return 0.0f;
}
float YingyangAudioProcessor::dbToGain (float db) noexcept { return std::pow (10.0f, db / 20.0f); }
float YingyangAudioProcessor::softClip (float x, float drive) noexcept
{
    const float d = juce::jmax (0.1f, drive);
    const float norm = std::tanh (d);
    return norm > 0.0001f ? std::tanh (x * d) / norm : x;
}

void YingyangAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    lowState = 0.0f;
    highState = 0.0f;
    deEssLow = 0.0f;
    fetEnv = 0.0f;
    optoEnv = 0.0f;
    noiseEnv = 0.0f;
    deEssEnv = 0.0f;
}
void YingyangAudioProcessor::releaseResources() {}

bool YingyangAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void YingyangAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int n = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, n);

    const float bassDb = get ("BASS"), midDb = get ("MID"), trebleDb = get ("TREBLE");
    const float sat = get ("SAT") / 100.0f;
    const float blend = get ("BLEND") / 100.0f;
    const float threshold = get ("THRESH");
    const float ratio = juce::jmax (1.0f, get ("RATIO"));
    const float attackMs = get ("ATTACK");
    const float releaseMs = get ("RELEASE");
    const float deEss = get ("DEESS") / 100.0f;
    const float noise = get ("NOISE") / 100.0f;
    const float outGain = dbToGain (get ("OUTPUT"));

    const float lowA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 180.0f / static_cast<float> (sr));
    const float midA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 1100.0f / static_cast<float> (sr));
    const float fetAttack = smoothCoeff (juce::jmax (0.1f, attackMs * 0.55f), sr);
    const float fetRelease = smoothCoeff (juce::jmax (5.0f, releaseMs * 0.45f), sr);
    const float optoAttack = smoothCoeff (juce::jmax (12.0f, attackMs * 3.0f), sr);
    const float optoRelease = smoothCoeff (juce::jmax (100.0f, releaseMs * 2.2f), sr);
    const float noiseAttack = smoothCoeff (8.0f, sr);
    const float noiseRelease = smoothCoeff (90.0f, sr);
    const float deAttack = smoothCoeff (0.7f, sr);
    const float deRelease = smoothCoeff (65.0f, sr);
    const float deA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * 5200.0f / static_cast<float> (sr));
    const float lowGain = dbToGain (bassDb);
    const float midGain = dbToGain (midDb);
    const float highGain = dbToGain (trebleDb);
    const float drive = 1.0f + sat * 5.5f;

    for (int ch = 0; ch < channels; ++ch)
    {
        float* x = buffer.getWritePointer (ch);
        for (int i = 0; i < n; ++i)
        {
            const float input = x[i];
            lowState += lowA * (input - lowState);
            highState += midA * (input - highState);
            const float low = lowState;
            const float mid = highState - lowState;
            const float high = input - highState;
            float toned = low * lowGain + mid * midGain + high * highGain;
            
            if (sat > 0.0001f)
                toned = toned * (1.0f - sat * 0.20f) + softClip (toned, drive) * (sat * 0.42f);

            const float level = std::abs (toned);
            const float fetC = level > fetEnv ? fetAttack : fetRelease;
            const float optoC = level > optoEnv ? optoAttack : optoRelease;
            fetEnv = fetC * fetEnv + (1.0f - fetC) * level;
            optoEnv = optoC * optoEnv + (1.0f - optoC) * level;
            const float fetDb = 20.0f * std::log10 (juce::jmax (1.0e-6f, fetEnv));
            const float optoDb = 20.0f * std::log10 (juce::jmax (1.0e-6f, optoEnv));
            const float overFet = juce::jmax (0.0f, fetDb - threshold);
            const float overOpto = juce::jmax (0.0f, optoDb - threshold);
            const float grFet = overFet * (1.0f - 1.0f / ratio);
            const float optoRatio = 1.0f + (ratio - 1.0f) * 0.72f;
            const float grOpto = overOpto * (1.0f - 1.0f / optoRatio);
            const float grDb = grFet * (1.0f - blend) + grOpto * blend;
            float y = toned * dbToGain (-grDb);

            deEssLow += deA * (y - deEssLow);
            const float highBand = y - deEssLow;
            const float sib = std::abs (highBand);
            const float dc = sib > deEssEnv ? deAttack : deRelease;
            deEssEnv = dc * deEssEnv + (1.0f - dc) * sib;
            const float sibDb = 20.0f * std::log10 (juce::jmax (1.0e-6f, deEssEnv));
            const float sibOver = juce::jmax (0.0f, sibDb - (-30.0f));
            const float deAttenDb = juce::jmin (12.0f, sibOver * deEss * 0.55f);
            y = deEssLow + highBand * dbToGain (-deAttenDb);

            const float nc = std::abs (y) > noiseEnv ? noiseAttack : noiseRelease;
            noiseEnv = nc * noiseEnv + (1.0f - nc) * std::abs (y);
            const float floor = dbToGain (-66.0f + noise * 22.0f);
            float noiseGain = 1.0f;
            if (noise > 0.0001f && noiseEnv < floor)
            {
                const float relative = juce::jlimit (0.0f, 1.0f, noiseEnv / juce::jmax (floor, 1.0e-6f));
                noiseGain = 1.0f - noise * (1.0f - relative) * 0.88f;
            }
            x[i] = y * noiseGain * outGain;
        }
    }
}

// Buradaki 'juce::AudioProcessorEditor*' artık .h dosyanla tamamen aynı!
juce::AudioProcessorEditor* YingyangAudioProcessor::createEditor() 
{ 
    return new YingyangAudioProcessorEditor (*this); 
}

void YingyangAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}

void YingyangAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() 
{ 
    return new YingyangAudioProcessor(); 
}
