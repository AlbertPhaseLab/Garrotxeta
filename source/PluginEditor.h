#pragma once

#include "PluginProcessor.h"

// --- Custom Look And Feel ---
class GarrotxetaLookAndFeel : public juce::LookAndFeel_V4 {
public:
    GarrotxetaLookAndFeel();
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                          float pos, float start, float end, juce::Slider& s) override;
    
    juce::Label* createSliderTextBox(juce::Slider& s) override;
    
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

private:
    juce::Image knobImage;
};

// --- Editor Class ---
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    void setupControl(juce::Slider& s, juce::Label& l, juce::String name, juce::String paramID, bool showBox);

    AudioPluginAudioProcessor& audioProcessor;
    GarrotxetaLookAndFeel lookAndFeel;

    // Images
    juce::Image backgroundImage;
    juce::Image logoImage;

    // Controls
    juce::Slider gainSlider, freqSlider, outSlider;
    juce::Slider bassSlider, midSlider, trebleSlider;
    juce::Label gainL, freqL, outL;
    juce::Label bassL, midL, trebleL;

    float ledBrightness = 0.0f;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<Attachment>> attachments;

    juce::TextButton languageButton;
    bool isCatalan = true;
    void updateLanguage();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};