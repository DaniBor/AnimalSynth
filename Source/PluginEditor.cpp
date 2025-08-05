/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include <BinaryData.h>

//==============================================================================
AnimalSynthAudioProcessorEditor::AnimalSynthAudioProcessorEditor (AnimalSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.


    /// @warning The AudioScope has to be set up BEFORE setting the size of the Plugin window otherwise it crashes! DO NOT MOVE THIS!
    audioScope = std::make_unique<ScaledVisualiserComponent>(1024);
    audioProcessor.pushAudioToScope = [this](const juce::AudioBuffer<float>& b)
    {
        if(audioScope)
            audioScope->pushBuffer(b);
    };
    addAndMakeVisible(*audioScope);

    setSize (500, 375);

    setLookAndFeel(&customLookAndFeel);

    auto& par = audioProcessor.parameters;

    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_jpg, BinaryData::background_jpgSize);

    sineImage = juce::ImageCache::getFromMemory(BinaryData::wolfPanel_jpg, BinaryData::wolfPanel_jpgSize);
    sawImage = juce::ImageCache::getFromMemory(BinaryData::bearPanel_jpg, BinaryData::bearPanel_jpgSize);
    squareImage = juce::ImageCache::getFromMemory(BinaryData::dogPanel_jpg, BinaryData::dogPanel_jpgSize);
    triangleImage = juce::ImageCache::getFromMemory(BinaryData::birdPanel_jpg, BinaryData::birdPanel_jpgSize);

    waveformSelector.addItem("Howl (Sine)", 1);
    waveformSelector.addItem("Growl (Saw)", 2);
    waveformSelector.addItem("Bark (Square)", 3);
    waveformSelector.addItem("Chirp (Triangle)", 4);
    waveformSelector.onChange = [this] { updateEffectUI(); };
    addAndMakeVisible(waveformSelector);

    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.parameters, "waveform", waveformSelector);

    auto styleKnob = [](juce::Slider& s)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        };

    styleKnob(attackSlider); addAndMakeVisible(attackSlider);
    styleKnob(decaySlider);  addAndMakeVisible(decaySlider);
    styleKnob(sustainSlider); addAndMakeVisible(sustainSlider);
    styleKnob(releaseSlider); addAndMakeVisible(releaseSlider);

    attackAttachment = std::make_unique<SliderAttachment>(par, "attack", attackSlider);
    decayAttachment = std::make_unique<SliderAttachment>(par, "decay", decaySlider);
    sustainAttachment = std::make_unique<SliderAttachment>(par, "sustain", sustainSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(par, "release", releaseSlider);

    sineFXPanel.setImage(sineImage);
    addAndMakeVisible(sineFXPanel);

    sawFXPanel.setImage(sawImage);
    addAndMakeVisible(sawFXPanel);

    squareFXPanel.setImage(squareImage);
    addAndMakeVisible(squareFXPanel);

    triangleFXPanel.setImage(triangleImage);
    addAndMakeVisible(triangleFXPanel);

    wildlifeCam.setNewAnimal(0);
    logoPanel.setNewAnimal(99);

    wildlifeCam.setInterceptsMouseClicks(false, false);
    wildlifeCam.setText("Animation Placeholder");
    addAndMakeVisible(wildlifeCam);

    setupEffectPanels();

    logoPanel.setText("PolyMal");
    addAndMakeVisible(logoPanel);

#pragma region Slider Text Overrides

    // Saw Shape text display — show "Off" if drive is OFF
    sawShapeSlider.textFromValueFunction = [this](double value) {
        return (sawDriveSlider.getValue() <= 0.9) ? juce::String("Off") : juce::String(value, 1);
    };

    // Saw Drive text display
    sawDriveSlider.textFromValueFunction = [](double value) {
        return (value <= 0.9) ? juce::String("Off") : juce::String(value, 1);
    };
    // Force Shape Slider to show "Off" if drive if off
    sawDriveSlider.onValueChange = [this]() {
        sawShapeSlider.setTextValueSuffix(""); // optional, reset suffix
        sawShapeSlider.updateText();           // force update display
    };

#pragma endregion



}

AnimalSynthAudioProcessorEditor::~AnimalSynthAudioProcessorEditor()
{
    audioProcessor.pushAudioToScope = nullptr;
}

//==============================================================================
void AnimalSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    if (backgroundImage.isValid())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll(juce::Colours::black); // Fallback color
}

void AnimalSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // Top bar for waveform selector and animation preview
    auto topBar = bounds.removeFromTop(180); // Reserve height for both


    // Reserve left and right portions
    auto waveformColumn = topBar.removeFromLeft(220); // 200px wide (left side)
    auto animationBounds = topBar.removeFromRight(220).withHeight(160); // animation right side

    waveformSelector.setBounds(waveformColumn.removeFromTop(40));
    waveformColumn.removeFromTop(10);
    audioScope->setBounds(waveformColumn.removeFromTop(110));
    
    wildlifeCam.setBounds(animationBounds);
    
    // FX panel area
    auto fxBounds = bounds.removeFromTop(100);
    sineFXPanel.setBounds(fxBounds);
    sawFXPanel.setBounds(fxBounds);
    squareFXPanel.setBounds(fxBounds);
    triangleFXPanel.setBounds(fxBounds);

    // Reserve bottom area for ADSR
    auto adsrAreaFull = bounds.removeFromBottom(60);

    // ADSR and Logo area
    auto adsrArea = adsrAreaFull.removeFromLeft(adsrAreaFull.getWidth() * 3 / 4);
    auto adsrPlaceholderArea = adsrAreaFull; // Remaining 25%

    // Layout ADSR sliders in adsrArea
    auto sliderZoneWidth = adsrArea.getWidth() / 4;

    for (auto* slider : { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider })
    {
        auto zone = adsrArea.removeFromLeft(sliderZoneWidth);
        slider->setBounds(zone.withSizeKeepingCentre(50, 60)); // smaller knob
    }

    // Reuse animation component as second placeholder
    logoPanel.setBounds(adsrPlaceholderArea);

    // FX sliders
    const int fxSliderSize = 60;
    int sliderPadding = 20;
    int yPos = 35;
    int titleLabelOffset = 40;
    int titleLabelYPos = -20;

    // === Sine Sliders ===
    vibratoDepthSlider.setBounds(10, yPos, fxSliderSize, fxSliderSize);
    vibratoRateSlider.setBounds(10 + sliderPadding + fxSliderSize, yPos, fxSliderSize, fxSliderSize);
    vibratoLabel.setBounds(10 + titleLabelOffset, titleLabelYPos, 60, 60);

    chorusDepthSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 2, yPos, fxSliderSize, fxSliderSize);
    chorusRateSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 3, yPos, fxSliderSize, fxSliderSize);
    chorusLabel.setBounds(10 + titleLabelOffset * 5, titleLabelYPos, 60, 60);

    tremoloDepthSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 4, yPos, fxSliderSize, fxSliderSize);
    tremoloRateSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 5, yPos, fxSliderSize, fxSliderSize);
    tremoloLabel.setBounds(10 + titleLabelOffset * 9, titleLabelYPos, 60, 60);

    // === Saw Sliders ===
    sawCombTimeSlider.setBounds(10, yPos, fxSliderSize, fxSliderSize);
    sawCombFeedbackSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 1, yPos, fxSliderSize, fxSliderSize);

    formantFreqSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 2, yPos, fxSliderSize, fxSliderSize);
    formantResSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 3, yPos, fxSliderSize, fxSliderSize);

    sawDriveSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 4, yPos, fxSliderSize, fxSliderSize);
    sawShapeSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 5, yPos, fxSliderSize, fxSliderSize);


    // === Square Sliders ===
    squarePunchAmountSlider.setBounds(10, yPos, fxSliderSize, fxSliderSize);
    squarePunchDecaySlider.setBounds(10 + (sliderPadding + fxSliderSize) * 1, yPos, fxSliderSize, fxSliderSize);

    squareBitcrushRateSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 2, yPos, fxSliderSize, fxSliderSize);
    squareBitcrushDepthSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 3, yPos, fxSliderSize, fxSliderSize);

    barkFilterFreqSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 4, yPos, fxSliderSize, fxSliderSize);
    barkFilterResSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 5, yPos, fxSliderSize, fxSliderSize);

    

    // === Triangle Sliders ===
    triGlideTimeSlider.setBounds(10, yPos, fxSliderSize, fxSliderSize);
    triGlideDepthSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 1, yPos, fxSliderSize, fxSliderSize);

    triChirpRateSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 2, yPos, fxSliderSize, fxSliderSize);
    triChirpDepthSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 3, yPos, fxSliderSize, fxSliderSize);

    triEchoMixSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 4, yPos, fxSliderSize, fxSliderSize);
    triEchoTimeSlider.setBounds(10 + (sliderPadding + fxSliderSize) * 5, yPos, fxSliderSize, fxSliderSize);

}

/**
 * @brief Wechselt zwischen den Panels wenn ein neues ausgewählt wird
 */
void AnimalSynthAudioProcessorEditor::updateEffectUI()
{
    sineFXPanel.setVisible(false);
    sawFXPanel.setVisible(false);
    squareFXPanel.setVisible(false);
    triangleFXPanel.setVisible(false);

    switch (waveformSelector.getSelectedId())
    {
    case 1: sineFXPanel.setVisible(true); break;
    case 2: sawFXPanel.setVisible(true); break;
    case 3: squareFXPanel.setVisible(true); break;
    case 4: triangleFXPanel.setVisible(true); break;
    }

    repaint();
}

/**
 * @brief Bereitet alle Slider und Labels der FX Panele vor
 */
void AnimalSynthAudioProcessorEditor::setupEffectPanels()
{
    // ===== Sine Panel =====
    // === Vibrato ===
    vibratoLabel.setText("Vibrato", juce::dontSendNotification);
    vibratoLabel.setJustificationType(juce::Justification::centred);
    sineFXPanel.addAndMakeVisible(vibratoLabel);

    // Vibrato Rate
    vibratoRateSlider.setSliderStyle(juce::Slider::Rotary);
    vibratoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    vibratoRateLabel.setText("Rate", juce::dontSendNotification);
    vibratoRateLabel.setJustificationType(juce::Justification::centred);
    vibratoRateLabel.attachToComponent(&vibratoRateSlider, false);
    sineFXPanel.addAndMakeVisible(vibratoRateSlider);
    sineFXPanel.addAndMakeVisible(vibratoRateLabel);

    vibratoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "vibratoRate", vibratoRateSlider);

    // Vibrato Depth
    vibratoDepthSlider.setSliderStyle(juce::Slider::Rotary);
    vibratoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    vibratoDepthLabel.setText("Depth", juce::dontSendNotification);
    vibratoDepthLabel.setJustificationType(juce::Justification::centred);
    vibratoDepthLabel.attachToComponent(&vibratoDepthSlider, false);
    sineFXPanel.addAndMakeVisible(vibratoDepthSlider);
    sineFXPanel.addAndMakeVisible(vibratoDepthLabel);

    vibratoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "vibratoDepth", vibratoDepthSlider);


    // === Flutter ===
    chorusLabel.setText("Chorus", juce::dontSendNotification);
    chorusLabel.setJustificationType(juce::Justification::centred);
    sineFXPanel.addAndMakeVisible(chorusLabel);
    // Flutter Depth
    chorusDepthSlider.setSliderStyle(juce::Slider::Rotary);
    chorusDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    chorusDepthLabel.setText("Depth", juce::dontSendNotification);
    chorusDepthLabel.setJustificationType(juce::Justification::centred);
    chorusDepthLabel.attachToComponent(&chorusDepthSlider, false);
    sineFXPanel.addAndMakeVisible(chorusDepthSlider);
    sineFXPanel.addAndMakeVisible(chorusDepthLabel);

    chorusDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "sineChorusDepth", chorusDepthSlider);

    // Flutter Rate
    chorusRateSlider.setSliderStyle(juce::Slider::Rotary);
    chorusRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    chorusRateLabel.setText("Rate", juce::dontSendNotification);
    chorusRateLabel.setJustificationType(juce::Justification::centred);
    chorusRateLabel.attachToComponent(&chorusRateSlider, false);
    sineFXPanel.addAndMakeVisible(chorusRateSlider);
    sineFXPanel.addAndMakeVisible(chorusRateLabel);

    chorusRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "sineChorusRate", chorusRateSlider);

    // === Tremolo ===
    tremoloLabel.setText("Tremolo", juce::dontSendNotification);
    tremoloLabel.setJustificationType(juce::Justification::centred);
    sineFXPanel.addAndMakeVisible(tremoloLabel);
    //Tremolo Depth
    tremoloDepthSlider.setSliderStyle(juce::Slider::Rotary);
    tremoloDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    tremoloDepthLabel.setText("Depth", juce::dontSendNotification);
    tremoloDepthLabel.setJustificationType(juce::Justification::centred);
    tremoloDepthLabel.attachToComponent(&tremoloDepthSlider, false);
    sineFXPanel.addAndMakeVisible(tremoloDepthSlider);
    sineFXPanel.addAndMakeVisible(tremoloDepthLabel);

    tremoloDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "tremoloDepth", tremoloDepthSlider);

    //Tremolo Rate
    tremoloRateSlider.setSliderStyle(juce::Slider::Rotary);
    tremoloRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    tremoloRateLabel.setText("Rate", juce::dontSendNotification);
    tremoloRateLabel.setJustificationType(juce::Justification::centred);
    tremoloRateLabel.attachToComponent(&tremoloRateSlider, false);
    sineFXPanel.addAndMakeVisible(tremoloRateSlider);
    sineFXPanel.addAndMakeVisible(tremoloRateLabel);

    tremoloRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "tremoloRate", tremoloRateSlider);

    // ===== Saw Panel =====
    // Comb Time
    sawCombTimeSlider.setSliderStyle(juce::Slider::Rotary);
    sawCombTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    sawCombTimeLabel.setText("Comb Time", juce::dontSendNotification);
    sawCombTimeLabel.attachToComponent(&sawCombTimeSlider, false);
    sawFXPanel.addAndMakeVisible(sawCombTimeSlider);
    sawFXPanel.addAndMakeVisible(sawCombTimeLabel);

    sawCombTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "sawCombTime", sawCombTimeSlider);

    // Comb Feedback
    sawCombFeedbackSlider.setSliderStyle(juce::Slider::Rotary);
    sawCombFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    sawCombFeedbackLabel.setText("Comb Feedback", juce::dontSendNotification);
    sawCombFeedbackLabel.attachToComponent(&sawCombFeedbackSlider, false);
    sawFXPanel.addAndMakeVisible(sawCombFeedbackSlider);
    sawFXPanel.addAndMakeVisible(sawCombFeedbackLabel);

    sawCombFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "sawCombFeedback", sawCombFeedbackSlider);

    formantFreqSlider.setSliderStyle(juce::Slider::Rotary);
    formantFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    formantFreqLabel.setText("Formant Freq", juce::dontSendNotification);
    formantFreqLabel.attachToComponent(&formantFreqSlider, false);
    sawFXPanel.addAndMakeVisible(formantFreqSlider);
    sawFXPanel.addAndMakeVisible(formantResSlider);


    formantResSlider.setSliderStyle(juce::Slider::Rotary);
    formantResSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    formantResLabel.setText("Formant Res", juce::dontSendNotification);
    formantResLabel.attachToComponent(&formantResSlider, false);
    sawFXPanel.addAndMakeVisible(formantFreqLabel);
    sawFXPanel.addAndMakeVisible(formantResLabel);

    formantFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "formantFreq", formantFreqSlider);

    formantResAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "formantResonance", formantResSlider);

    // === Saw Drive ===
    sawDriveSlider.setSliderStyle(juce::Slider::Rotary);
    sawDriveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    sawDriveLabel.setText("Drive", juce::dontSendNotification);
    sawDriveLabel.attachToComponent(&sawDriveSlider, false);
    sawFXPanel.addAndMakeVisible(sawDriveSlider);
    sawFXPanel.addAndMakeVisible(sawDriveLabel);

    sawDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, "sawDrive", sawDriveSlider);

    // === Saw Shape ===
    sawShapeSlider.setSliderStyle(juce::Slider::Rotary);
    sawShapeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    sawShapeLabel.setText("Shape", juce::dontSendNotification);
    sawShapeLabel.attachToComponent(&sawShapeSlider, false);
    sawFXPanel.addAndMakeVisible(sawShapeSlider);
    sawFXPanel.addAndMakeVisible(sawShapeLabel);

    sawShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "sawShape", sawShapeSlider);


    // ===== Square Panel =====
    // Punch Amount Slider
    squarePunchAmountSlider.setSliderStyle(juce::Slider::Rotary);
    squarePunchAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    squarePunchAmountLabel.setText("Punch Amt", juce::dontSendNotification);
    squarePunchAmountLabel.attachToComponent(&squarePunchAmountSlider, false);
    squareFXPanel.addAndMakeVisible(squarePunchAmountSlider);
    squareFXPanel.addAndMakeVisible(squarePunchAmountLabel);

    squarePunchAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "squarePunchAmount", squarePunchAmountSlider);

    // Punch Decay Slider
    squarePunchDecaySlider.setSliderStyle(juce::Slider::Rotary);
    squarePunchDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    squarePunchDecayLabel.setText("Punch Decay", juce::dontSendNotification);
    squarePunchDecayLabel.attachToComponent(&squarePunchDecaySlider, false);
    squareFXPanel.addAndMakeVisible(squarePunchDecaySlider);
    squareFXPanel.addAndMakeVisible(squarePunchDecayLabel);

    squarePunchDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "squarePunchDecay", squarePunchDecaySlider);

    // Rate (Sample Rate Reduction)
    squareBitcrushRateSlider.setSliderStyle(juce::Slider::Rotary);
    squareBitcrushRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    squareBitcrushRateLabel.setText("Crush Rate", juce::dontSendNotification);
    squareBitcrushRateLabel.attachToComponent(&squareBitcrushRateSlider, false);
    squareFXPanel.addAndMakeVisible(squareBitcrushRateSlider);
    squareFXPanel.addAndMakeVisible(squareBitcrushRateLabel);

    squareBitcrushRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "squareBitcrushRate", squareBitcrushRateSlider);

    // Depth (Bit Depth Reduction)
    squareBitcrushDepthSlider.setSliderStyle(juce::Slider::Rotary);
    squareBitcrushDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    squareBitcrushDepthLabel.setText("Crush Depth", juce::dontSendNotification);
    squareBitcrushDepthLabel.attachToComponent(&squareBitcrushDepthSlider, false);
    squareFXPanel.addAndMakeVisible(squareBitcrushDepthSlider);
    squareFXPanel.addAndMakeVisible(squareBitcrushDepthLabel);

    squareBitcrushDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "squareBitcrushDepth", squareBitcrushDepthSlider);

    barkFilterFreqSlider.setSliderStyle(juce::Slider::Rotary);
    barkFilterFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    barkFilterFreqLabel.setText("Bark Freq", juce::dontSendNotification);
    barkFilterFreqLabel.attachToComponent(&barkFilterFreqSlider, false);
    squareFXPanel.addAndMakeVisible(barkFilterFreqSlider);
    squareFXPanel.addAndMakeVisible(barkFilterFreqLabel);

    barkFilterFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "barkFilterFreq", barkFilterFreqSlider);

    barkFilterResSlider.setSliderStyle(juce::Slider::Rotary);
    barkFilterResSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    barkFilterResLabel.setText("Bark Res", juce::dontSendNotification);
    barkFilterResLabel.attachToComponent(&barkFilterResSlider, false);
    squareFXPanel.addAndMakeVisible(barkFilterResSlider);
    squareFXPanel.addAndMakeVisible(barkFilterResLabel);

    barkFilterResAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "barkFilterResonance", barkFilterResSlider);


    // ===== Triangle Panel =====
    // Glide Time
    triGlideTimeSlider.setSliderStyle(juce::Slider::Rotary);
    triGlideTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    triGlideTimeLabel.setText("Glide Time", juce::dontSendNotification);
    triGlideTimeLabel.attachToComponent(&triGlideTimeSlider, false);
    triangleFXPanel.addAndMakeVisible(triGlideTimeSlider);
    triangleFXPanel.addAndMakeVisible(triGlideTimeLabel);

    triGlideTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "triGlideTime", triGlideTimeSlider);

    // Glide Depth
    triGlideDepthSlider.setSliderStyle(juce::Slider::Rotary);
    triGlideDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    triGlideDepthLabel.setText("Glide Depth", juce::dontSendNotification);
    triGlideDepthLabel.attachToComponent(&triGlideDepthSlider, false);
    triangleFXPanel.addAndMakeVisible(triGlideDepthSlider);
    triangleFXPanel.addAndMakeVisible(triGlideDepthLabel);
    triGlideDepthSlider.setNumDecimalPlacesToDisplay(0);

    triGlideDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "triGlideDepth", triGlideDepthSlider);

    // Chirp Rate
    triChirpRateSlider.setSliderStyle(juce::Slider::Rotary);
    triChirpRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    triChirpRateLabel.setText("Chirp Rate", juce::dontSendNotification);
    triChirpRateLabel.attachToComponent(&triChirpRateSlider, false);
    triangleFXPanel.addAndMakeVisible(triChirpRateSlider);
    triangleFXPanel.addAndMakeVisible(triChirpRateLabel);

    triChirpRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "triChirpRate", triChirpRateSlider);

    // Chirp Depth
    triChirpDepthSlider.setSliderStyle(juce::Slider::Rotary);
    triChirpDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    triChirpDepthLabel.setText("Chirp Depth", juce::dontSendNotification);
    triChirpDepthLabel.attachToComponent(&triChirpDepthSlider, false);
    triangleFXPanel.addAndMakeVisible(triChirpDepthSlider);
    triangleFXPanel.addAndMakeVisible(triChirpDepthLabel);

    triChirpDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "triChirpDepth", triChirpDepthSlider);

    // Echo Time
    triEchoTimeSlider.setSliderStyle(juce::Slider::Rotary);
    triEchoTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    triEchoTimeLabel.setText("Echo Time", juce::dontSendNotification);
    triEchoTimeLabel.attachToComponent(&triEchoTimeSlider, false);
    triangleFXPanel.addAndMakeVisible(triEchoTimeSlider);
    triangleFXPanel.addAndMakeVisible(triEchoTimeLabel);

    triEchoTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "triEchoTime", triEchoTimeSlider);

    // Echo Mix
    triEchoMixSlider.setSliderStyle(juce::Slider::Rotary);
    triEchoMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    triEchoMixLabel.setText("Echo Mix", juce::dontSendNotification);
    triEchoMixLabel.attachToComponent(&triEchoMixSlider, false);
    triangleFXPanel.addAndMakeVisible(triEchoMixSlider);
    triangleFXPanel.addAndMakeVisible(triEchoMixLabel);

    triEchoMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "triEchoMix", triEchoMixSlider);

    updateEffectUI();
}


