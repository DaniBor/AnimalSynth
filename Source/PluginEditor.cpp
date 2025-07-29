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

    setSize (400, 300);

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


    setupEffectPanels();

    // Attach sliders to parameters
    vibratoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "vibratoRate", vibratoRateSlider);

    vibratoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "vibratoDepth", vibratoDepthSlider);




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
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds().reduced(10);
    auto header = bounds.removeFromTop(40);

    waveformSelector.setBounds(header.removeFromLeft(200));

    sineFXPanel.setBounds(bounds);
    sawFXPanel.setBounds(bounds);
    squareFXPanel.setBounds(bounds);
    triangleFXPanel.setBounds(bounds);
    //audioScope->setBounds(10, 40, getWidth() - 20, 75);


    // Reserve bottom area for ADSR (e.g., 120 px high)
    auto adsrArea = bounds.removeFromBottom(80);

    // Layout ADSR sliders in adsrArea
    auto sliderZoneWidth = adsrArea.getWidth() / 4;

    for (auto* slider : { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider })
    {
        auto zone = adsrArea.removeFromLeft(sliderZoneWidth);
        slider->setBounds(zone.withSizeKeepingCentre(60, 80)); // smaller knob
    }

    // Remaining "bounds" is now for FX panel
    sineFXPanel.setBounds(bounds);
    sawFXPanel.setBounds(bounds);
    squareFXPanel.setBounds(bounds);
    triangleFXPanel.setBounds(bounds);

    // Define consistent slider size and padding
    const int fxSliderSize = 80;
    const int fxSliderPadding = 30;

    // Position sliders at top-left corner of their respective panels
    auto topLeft = juce::Rectangle<int>(fxSliderPadding, fxSliderPadding, fxSliderSize, fxSliderSize);

    // Position vibrato sliders in top-left area of sineFXPanel
    const int sliderSize = 80;
    const int spacing = 10;

    vibratoRateSlider.setBounds(spacing, spacing, sliderSize, sliderSize);
    vibratoDepthSlider.setBounds(spacing + sliderSize + spacing, spacing, sliderSize, sliderSize);

    sawSlider.setBounds(topLeft);
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
    // Sine Panel
    // Vibrato Rate
    vibratoRateSlider.setSliderStyle(juce::Slider::Rotary);
    vibratoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    vibratoRateLabel.setText("Vibrato Rate", juce::dontSendNotification);
    vibratoRateLabel.attachToComponent(&vibratoRateSlider, false);
    sineFXPanel.addAndMakeVisible(vibratoRateSlider);
    sineFXPanel.addAndMakeVisible(vibratoRateLabel);

    // Vibrato Depth
    vibratoDepthSlider.setSliderStyle(juce::Slider::Rotary);
    vibratoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    vibratoDepthLabel.setText("Vibrato Depth", juce::dontSendNotification);
    vibratoDepthLabel.attachToComponent(&vibratoDepthSlider, false);
    sineFXPanel.addAndMakeVisible(vibratoDepthSlider);
    sineFXPanel.addAndMakeVisible(vibratoDepthLabel);

    


    // Saw Panel
    sawSlider.setSliderStyle(juce::Slider::Rotary);
    sawSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    sawLabel.setText("Distortion Drive", juce::dontSendNotification);
    sawLabel.attachToComponent(&sawSlider, false);
    sawFXPanel.addAndMakeVisible(sawSlider);
    sawFXPanel.addAndMakeVisible(sawLabel);

    // Square Panel
    squareSlider.setSliderStyle(juce::Slider::Rotary);
    squareSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    squareLabel.setText("Croak Rate", juce::dontSendNotification);
    squareLabel.attachToComponent(&squareSlider, false);
    squareFXPanel.addAndMakeVisible(squareSlider);
    squareFXPanel.addAndMakeVisible(squareLabel);

    // Triangle Panel
    triangleSlider.setSliderStyle(juce::Slider::Rotary);
    triangleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    triangleLabel.setText("Reverb Mix", juce::dontSendNotification);
    triangleLabel.attachToComponent(&triangleSlider, false);
    triangleFXPanel.addAndMakeVisible(triangleSlider);
    triangleFXPanel.addAndMakeVisible(triangleLabel);


    updateEffectUI();
}


