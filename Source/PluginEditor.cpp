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

    waveformSelector.addItem("Sine", 1);
    waveformSelector.addItem("Saw", 2);
    waveformSelector.addItem("Square", 3);
    waveformSelector.addItem("Triangle", 4);
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

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AnimalSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    waveformSelector.setBounds(10, 10, 150, 25);
    //audioScope->setBounds(10, 40, getWidth() - 20, 75);


    auto bounds = getLocalBounds().reduced(10);
    auto row = bounds.removeFromBottom(100);

    attackSlider.setBounds(row.removeFromLeft(100));
    decaySlider.setBounds(row.removeFromLeft(100));
    sustainSlider.setBounds(row.removeFromLeft(100));
    releaseSlider.setBounds(row.removeFromLeft(100));

}
