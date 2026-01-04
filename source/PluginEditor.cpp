#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

// --- Look And Feel Implementation ---
GarrotxetaLookAndFeel::GarrotxetaLookAndFeel() {
    knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::orange);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void GarrotxetaLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                                           float pos, float start, float end, juce::Slider& s) 
{
    if (knobImage.isValid()) 
    {
        const float rotation = start + pos * (end - start);

        // 1. We explicitly convert the ints to floats to avoid the warning
        const float fx = static_cast<float>(x);
        const float fy = static_cast<float>(y);
        const float fw = static_cast<float>(width);
        const float fh = static_cast<float>(height);

        // 2. We define the limits and the center
        auto bounds = juce::Rectangle<float>(fx, fy, fw, fh).reduced(10.0f);
        auto centreX = fx + fw * 0.5f;
        auto centreY = fy + fh * 0.5f;
        
        // 3. The visual radius is the overall size we want for the knob
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;

        // --- TRIM ADJUSTMENT ---
        float trimAmount = 13.5f; // Edge trim
        float clippingRadius = radius - trimAmount;

        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.saveState();
        
        // Circular clipping mask (slightly smaller than the image)
        juce::Path clipPath;
        clipPath.addEllipse(centreX - clippingRadius, centreY - clippingRadius, 
                            clippingRadius * 2.0f, clippingRadius * 2.0f);
        g.reduceClipRegion(clipPath);

        // Safety background (black) in case the image has internal transparencies
        g.setColour(juce::Colours::black);
        g.fillEllipse(centreX - clippingRadius, centreY - clippingRadius, 
                      clippingRadius * 2.0f, clippingRadius * 2.0f);

        // --- IMAGE TRANSFORMATION ---
        // Use of 'radius' (the large one) for the scale, but the drawing will be cut off by the 'clippingRadius'
        float scale = 1.05f; // An extra 5% to ensure it covers the entire cut-out circle
        auto transform = juce::AffineTransform::rotation(rotation, 
                                static_cast<float>(knobImage.getWidth()) * 0.5f, 
                                static_cast<float>(knobImage.getHeight()) * 0.5f)
                            .scaled((radius * 2.0f * scale) / static_cast<float>(knobImage.getWidth()))
                            .translated(centreX - (radius * scale), centreY - (radius * scale));

        g.drawImageTransformed(knobImage, transform);
        g.restoreState();
        
        // --- FINAL AESTHETIC DETAIL ---
        // Drawing the integration highlight on the clipped edge
        g.setColour(juce::Colours::orange.withAlpha(0.2f));
        g.drawEllipse(centreX - clippingRadius, centreY - clippingRadius, 
                      clippingRadius * 2.0f, clippingRadius * 2.0f, 1.5f);
    } 
    else 
    {
        // Fallback in case the image does not load
        juce::LookAndFeel_V4::drawRotarySlider(g, x, y, width, height, pos, start, end, s);
    }
}

juce::Label* GarrotxetaLookAndFeel::createSliderTextBox(juce::Slider& s) {
    auto* l = LookAndFeel_V4::createSliderTextBox(s);
    l->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.7f));
    l->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    return l;
}

void GarrotxetaLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    if (dynamic_cast<juce::Slider*>(label.getParentComponent()) != nullptr) {
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(label.getLocalBounds().toFloat(), 4.0f);
        g.setColour(juce::Colours::orangered);
        g.fillRect(2, label.getHeight() - 2, label.getWidth() - 4, 2);
    }
    g.setFont(label.getFont());
    g.setColour(juce::Colours::orange);
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType(), true);
}

// --- Editor Implementation ---
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    logoImage = juce::ImageCache::getFromMemory(BinaryData::garrotxeta_png, BinaryData::garrotxeta_pngSize);

    setLookAndFeel(&lookAndFeel);

    setupControl(gainSlider, gainL, "GAIN", "gain", true);
    setupControl(freqSlider, freqL, "MID FREQ", "frequency", true);
    setupControl(outSlider, outL, "OUT", "out", true);
    setupControl(bassSlider, bassL, "BASS", "bass", true);
    setupControl(midSlider, midL, "MIDDLE", "middle", true);
    setupControl(trebleSlider, trebleL, "TREBLE", "treble", true);

    // Language Button Settings
    addAndMakeVisible(languageButton);
    languageButton.onClick = [this] { 
        isCatalan = !isCatalan; 
        updateLanguage(); 
    };
    
    // Quick aesthetic for the button
    languageButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black.withAlpha(0.5f));
    languageButton.setColour(juce::TextButton::textColourOffId, juce::Colours::orange);

    // Initialize texts (This will set the names to Catalan or English at startup)
    updateLanguage();

    startTimerHz(60);
    setSize (567, 550);
}

void AudioPluginAudioProcessorEditor::updateLanguage()
{
    if (isCatalan)
    {
        languageButton.setButtonText(juce::CharPointer_UTF8("CATALÀ")); // Nombre del botón en Catalán
        
        gainL.setText("FOC", juce::dontSendNotification);
        freqL.setText("TRO", juce::dontSendNotification);
        outL.setText("SORTIDA", juce::dontSendNotification);
        bassL.setText("TERRA", juce::dontSendNotification);
        midL.setText("NUCLI", juce::dontSendNotification);
        trebleL.setText("LLUM", juce::dontSendNotification);
    }
    else
    {
        languageButton.setButtonText(juce::CharPointer_UTF8("BÀRBAR")); // Nombre del botón en Inglés
        
        gainL.setText("DRIVE", juce::dontSendNotification);
        freqL.setText("THUNDER", juce::dontSendNotification);
        outL.setText("LEVEL", juce::dontSendNotification);
        bassL.setText("LOW", juce::dontSendNotification);
        midL.setText("CORE", juce::dontSendNotification);
        trebleL.setText("TOP", juce::dontSendNotification);
    }
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() { 
    setLookAndFeel(nullptr); 
}

void AudioPluginAudioProcessorEditor::setupControl(juce::Slider& s, juce::Label& l, juce::String name, juce::String paramID, bool showBox) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(showBox ? juce::Slider::TextBoxBelow : juce::Slider::NoTextBox, false, 70, 22);
    addAndMakeVisible(s);

    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
    addAndMakeVisible(l);

    attachments.push_back(std::make_unique<Attachment>(audioProcessor.apvts, paramID, s));
}

void AudioPluginAudioProcessorEditor::timerCallback() {
    ledBrightness = juce::jlimit(0.0f, 1.0f, audioProcessor.rmsLevel * 5.0f);
    repaint(); 
}

void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g) {
    if (backgroundImage.isValid()) {
        g.drawImageWithin(backgroundImage, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::fillDestination);
    } else {
        g.fillAll(juce::Colours::black);
    }

    // Draw the Volcanic Logo
    if (logoImage.isValid()) {
        auto logoArea = juce::Rectangle<int>(0, 20, getWidth(), 70);
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.drawImageWithin(logoImage, logoArea.getX(), logoArea.getY(), logoArea.getWidth(), logoArea.getHeight(), juce::RectanglePlacement::centred);
    }

    // LED status
    auto ledArea = juce::Rectangle<float>(35, 35, 12, 12); 
    g.setColour(juce::Colours::orangered.withAlpha(0.2f + (ledBrightness * 0.4f)));
    g.fillEllipse(ledArea.expanded(ledBrightness * 10.0f)); 
    g.setColour(juce::Colours::orange.withAlpha(0.6f + (ledBrightness * 0.4f)));
    g.fillEllipse(ledArea);
}

void AudioPluginAudioProcessorEditor::resized() {
    // 1. Position the language button in the top right corner
    // 90px width so that "CATALÀ" has enough space
    int btnWidth = 90;
    int btnHeight = 25;
    int margin = 15;
    languageButton.setBounds(getWidth() - btnWidth - margin, margin, btnWidth, btnHeight);

    auto area = getLocalBounds().reduced(20); 
    
    // 2. Space for the Logo
    area.removeFromTop(80);

    // 3. Divide the remaining space into two equal rows
    auto rowHeight = area.getHeight() / 2;
    auto topRow = area.removeFromTop(rowHeight);
    auto bottomRow = area;

    juce::Slider* topS[] = { &gainSlider, &freqSlider, &outSlider };
    juce::Label* topL[] = { &gainL, &freqL, &outL };
    juce::Slider* botS[] = { &bassSlider, &midSlider, &trebleSlider };
    juce::Label* botL[] = { &bassL, &midL, &trebleL };

    auto colWidth = area.getWidth() / 3;

    for (int i = 0; i < 3; ++i) {
        // --- Top Row ---
        // cT will contain the Slider and the Label of the top row
        auto cT = topRow.removeFromLeft(colWidth).reduced(5);
        
        // We place the Label below (20px)
        topL[i]->setBounds(cT.removeFromBottom(20)); 

        // The Slider occupies the remaining space. As it has TextBoxBelow,
        // the text box will be automatically drawn between the knob and the label.
        topS[i]->setBounds(cT); 

        // --- Bottom Row ---
        // cB will contain the Slider and the Label of the bottom row
        auto cB = bottomRow.removeFromLeft(colWidth).reduced(5);
        
        botL[i]->setBounds(cB.removeFromBottom(20)); 
        botS[i]->setBounds(cB); 
    }
}