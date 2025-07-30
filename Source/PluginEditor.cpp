/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AnimalSynthAudioProcessorEditor::AnimalSynthAudioProcessorEditor (AnimalSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    //audioScope = std::make_unique<ScaledVisualiserComponent>(2048);

    setSize (500, 375);

    setLookAndFeel(&customLookAndFeel);

    waveformSelector.addItem("Howl (Sine)", 1);
    waveformSelector.addItem("Growl (Saw)", 2);
    waveformSelector.addItem("Croak (Square)", 3);
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

    auto& par = audioProcessor.parameters;

    attackAttachment = std::make_unique<SliderAttachment>(par, "attack", attackSlider);
    decayAttachment = std::make_unique<SliderAttachment>(par, "decay", decaySlider);
    sustainAttachment = std::make_unique<SliderAttachment>(par, "sustain", sustainSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(par, "release", releaseSlider);

    addAndMakeVisible(sineFXPanel);
    addAndMakeVisible(sawFXPanel);
    addAndMakeVisible(squareFXPanel);
    addAndMakeVisible(triangleFXPanel);

    animationPlaceholder.setInterceptsMouseClicks(false, false);
    animationPlaceholder.setText("Animation Placeholder");
    addAndMakeVisible(animationPlaceholder);

    setupEffectPanels();

    polyMalButton.setText("PolyMal");
    addAndMakeVisible(polyMalButton);

    waveVisualizer.setText("Oscillator");
    addAndMakeVisible(waveVisualizer);

    // Attach sliders to parameters
    vibratoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "vibratoRate", vibratoRateSlider);

    vibratoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "vibratoDepth", vibratoDepthSlider);

    sawDistortionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "sawDistortionAmount", sawDistortionSlider);

    /*
    audioProcessor.pushAudioToScope = [this](const juce::AudioBuffer<float>& b)
        {
            audioScope->pushBuffer(b);
        };

    addAndMakeVisible(*audioScope);*/
}

AnimalSynthAudioProcessorEditor::~AnimalSynthAudioProcessorEditor()
{
}

//==============================================================================
void AnimalSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AnimalSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // Top bar for waveform selector and animation preview
    auto topBar = bounds.removeFromTop(180); // Reserve height for both


    // Reserve left and right portions
    auto waveformColumn = topBar.removeFromLeft(200); // 200px wide (left side)
    auto animationBounds = topBar.removeFromRight(220).withHeight(160); // animation right side

    waveformSelector.setBounds(waveformColumn.removeFromTop(40));
    waveformColumn.removeFromTop(10);
    waveVisualizer.setBounds(waveformColumn.removeFromTop(110));
    
    animationPlaceholder.setBounds(animationBounds);

    // FX panel area
    auto fxBounds = bounds.removeFromTop(100);
    sineFXPanel.setBounds(fxBounds);
    sawFXPanel.setBounds(fxBounds);
    squareFXPanel.setBounds(fxBounds);
    triangleFXPanel.setBounds(fxBounds);

    // Reserve bottom area for ADSR
    auto adsrAreaFull = bounds.removeFromBottom(60);

    // Split into 75% (for ADSR sliders) and 25% (for second visual placeholder)
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
    polyMalButton.setBounds(adsrPlaceholderArea);

    // FX sliders
    const int fxSliderSize = 60;
    const int fxSliderPadding = 15;

    auto topLeft = juce::Rectangle<int>(fxSliderPadding, fxSliderPadding, fxSliderSize, fxSliderSize);

    vibratoRateSlider.setBounds(10, 30, fxSliderSize, fxSliderSize);
    vibratoDepthSlider.setBounds(20 + fxSliderSize, 30, fxSliderSize, fxSliderSize);

    sawDistortionSlider.setBounds(10, 30, fxSliderSize, fxSliderSize);
    squareSlider.setBounds(topLeft);
    triangleSlider.setBounds(topLeft);
}


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


void AnimalSynthAudioProcessorEditor::setupEffectPanels()
{
    // === Sine Panel ===
    // Vibrato Rate
    vibratoRateSlider.setSliderStyle(juce::Slider::Rotary);
    vibratoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    vibratoRateLabel.setText("Vib Rate", juce::dontSendNotification);
    vibratoRateLabel.attachToComponent(&vibratoRateSlider, false);
    sineFXPanel.addAndMakeVisible(vibratoRateSlider);
    sineFXPanel.addAndMakeVisible(vibratoRateLabel);

    // Vibrato Depth
    vibratoDepthSlider.setSliderStyle(juce::Slider::Rotary);
    vibratoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    vibratoDepthLabel.setText("Vib Depth", juce::dontSendNotification);
    vibratoDepthLabel.attachToComponent(&vibratoDepthSlider, false);
    sineFXPanel.addAndMakeVisible(vibratoDepthSlider);
    sineFXPanel.addAndMakeVisible(vibratoDepthLabel);

    
    // === Saw Panel ===
    sawDistortionSlider.setSliderStyle(juce::Slider::Rotary);
    sawDistortionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    sawDistortionLabel.setText("Distortion", juce::dontSendNotification);
    sawDistortionLabel.attachToComponent(&sawDistortionSlider, false);
    sawFXPanel.addAndMakeVisible(sawDistortionSlider);
    sawFXPanel.addAndMakeVisible(sawDistortionLabel);


    // === Square Panel ===
    squareSlider.setSliderStyle(juce::Slider::Rotary);
    squareSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    squareLabel.setText("Croak Rate", juce::dontSendNotification);
    squareLabel.attachToComponent(&squareSlider, false);
    squareFXPanel.addAndMakeVisible(squareSlider);
    squareFXPanel.addAndMakeVisible(squareLabel);

    // === Triangle Panel ===
    triangleSlider.setSliderStyle(juce::Slider::Rotary);
    triangleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    triangleLabel.setText("Reverb Mix", juce::dontSendNotification);
    triangleLabel.attachToComponent(&triangleSlider, false);
    triangleFXPanel.addAndMakeVisible(triangleSlider);
    triangleFXPanel.addAndMakeVisible(triangleLabel);


    updateEffectUI();
}


