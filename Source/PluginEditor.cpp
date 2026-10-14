#include "PluginEditor.h"

namespace
{
    const juce::Colour bg (0xff111316), panel (0xff1a1d21), panel2 (0xff22262b), cream (0xffeee9df), muted (0xff9da3a8), amber (0xffd5a65a), teal (0xff71b7b0);
}

YingyangAudioProcessorEditor::Knob::Knob (YingyangAudioProcessor& p, const juce::String& id, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
    slider.setColour (juce::Slider::rotarySliderFillColourId, amber);
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, panel2);
    slider.setColour (juce::Slider::thumbColourId, cream);
    slider.setColour (juce::Slider::textBoxTextColourId, cream);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, panel);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);
    
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, muted);
    label.setFont (juce::Font (juce::FontOptions (11.5f).withStyle ("Bold")));
    addAndMakeVisible (label);

    // Güvenli ve akıllı hafıza bağlantısı kuruldu
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.parameters, id, slider);
}

void YingyangAudioProcessorEditor::Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (22));
    slider.setBounds (r);
}

YingyangAudioProcessorEditor::YingyangAudioProcessorEditor (YingyangAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (1040, 650);
    auto setupLabel = [] (juce::Label& l, const juce::String& text, float size, juce::Colour colour)
    {
        l.setText (text, juce::dontSendNotification);
        l.setFont (juce::Font (juce::FontOptions (size).withStyle ("Bold")));
        l.setColour (juce::Label::textColourId, colour);
        l.setJustificationType (juce::Justification::centredLeft);
    };
    setupLabel (title, "YINGYANG", 28.0f, cream);
    setupLabel (subtitle, "DUAL-CHARACTER VOCAL PROCESSOR  /  FET x OPTO", 10.5f, muted);
    setupLabel (toneHeading, "01   ANALOG TONE", 12.0f, amber);
    setupLabel (compHeading, "02   YIN / YANG COMPRESSOR", 12.0f, amber);
    setupLabel (cleanHeading, "03   CLEANUP", 12.0f, teal);
    setupLabel (outputHeading, "04   OUTPUT", 12.0f, teal);
    for (auto* l : { &title, &subtitle, &toneHeading, &compHeading, &cleanHeading, &outputHeading }) addAndMakeVisible (*l);

    bass   = std::make_unique<Knob> (p, "BASS", "BASS");
    mid    = std::make_unique<Knob> (p, "MID", "MID");
    treble = std::make_unique<Knob> (p, "TREBLE", "TREBLE");
    sat    = std::make_unique<Knob> (p, "SAT", "HARMONICS");
    blend  = std::make_unique<Knob> (p, "BLEND", "FET  /  OPTO");
    thresh = std::make_unique<Knob> (p, "THRESH", "THRESHOLD");
    ratio  = std::make_unique<Knob> (p, "RATIO", "RATIO");
    attack = std::make_unique<Knob> (p, "ATTACK", "ATTACK");
    release= std::make_unique<Knob> (p, "RELEASE", "RELEASE");
    deess  = std::make_unique<Knob> (p, "DEESS", "DE-ESS %");
    noise  = std::make_unique<Knob> (p, "NOISE", "NOISE REDUCTION");
    output = std::make_unique<Knob> (p, "OUTPUT", "OUTPUT GAIN");
    
    for (auto* k : { bass.get(), mid.get(), treble.get(), sat.get(), blend.get(), thresh.get(), ratio.get(), attack.get(), release.get(), deess.get(), noise.get(), output.get() })
    {
        addAndMakeVisible (*k);
    }
}

void YingyangAudioProcessorEditor::drawYinYang (juce::Graphics& g, juce::Rectangle<float> b)
{
    const float d = juce::jmin (b.getWidth(), b.getHeight());
    b = b.withSizeKeepingCentre (d, d);
    g.setColour (amber.withAlpha (0.16f)); g.fillEllipse (b.expanded (5.0f));
    g.setColour (cream); g.fillEllipse (b);
    auto left = b.withWidth (d * 0.5f);
    g.setColour (juce::Colour (0xff17191c)); g.fillRect (left);
    
    juce::Path s;
    s.startNewSubPath (b.getCentreX(), b.getY());
    s.cubicTo (b.getX() + d * 0.15f, b.getY() + d * 0.02f, b.getX() + d * 0.15f, b.getY() + d * 0.48f, b.getCentreX(), b.getCentreY());
    s.cubicTo (b.getRight() - d * 0.15f, b.getY() + d * 0.52f, b.getRight() - d * 0.15f, b.getBottom() - d * 0.02f, b.getCentreX(), b.getBottom());
    g.setColour (cream); g.strokePath (s, juce::PathStrokeType (d * 0.15f));
    g.setColour (amber); g.fillEllipse (b.getCentreX() - d * 0.09f, b.getY() + d * 0.25f, d * 0.18f, d * 0.18f);
    g.setColour (teal); g.fillEllipse (b.getCentreX() - d * 0.09f, b.getY() + d * 0.61f, d * 0.18f, d * 0.18f);
}

void YingyangAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    g.setColour (panel); g.fillRoundedRectangle (18.0f, 92.0f, 1004.0f, 214.0f, 14.0f);
    g.fillRoundedRectangle (18.0f, 320.0f, 1004.0f, 202.0f, 14.0f);
    g.fillRoundedRectangle (18.0f, 536.0f, 1004.0f, 96.0f, 14.0f);
    drawYinYang (g, { 922.0f, 18.0f, 54.0f, 54.0f });
    g.setColour (panel2); g.fillRoundedRectangle (986.0f, 24.0f, 30.0f, 42.0f, 7.0f);
    g.setColour (teal); g.fillEllipse (996.0f, 35.0f, 10.0f, 10.0f);
    g.setColour (muted); g.setFont (10.0f); g.drawText ("STEREO  /  ANALOG-INSPIRED DSP", 26, 625, 500, 14, juce::Justification::left);
}

void YingyangAudioProcessorEditor::resized()
{
    title.setBounds (24, 12, 350, 38);
    subtitle.setBounds (26, 49, 560, 20);
    toneHeading.setBounds (32, 98, 240, 20);
    compHeading.setBounds (32, 326, 300, 20);
    cleanHeading.setBounds (32, 542, 220, 20);
    outputHeading.setBounds (750, 542, 200, 20);

    const int kw = 112, kh = 160;
    bass->setBounds (38, 125, kw, kh); 
    mid->setBounds (164, 125, kw, kh); 
    treble->setBounds (290, 125, kw, kh); 
    sat->setBounds (416, 125, kw, kh);
    
    blend->setBounds (38, 354, kw, 150); 
    thresh->setBounds (164, 354, kw, 150); 
    ratio->setBounds (290, 354, kw, 150); 
    attack->setBounds (416, 354, kw, 150); 
    release->setBounds (542, 354, kw, 150);
    
    deess->setBounds (38, 562, kw, 68); 
    noise->setBounds (190, 562, 150, 68); 
    output->setBounds (748, 562, kw, 68);
}
