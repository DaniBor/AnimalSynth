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
    setSize (400, 300);

    waveformSelector.addItem("Sine", 1);
    waveformSelector.addItem("Saw", 2);
    waveformSelector.addItem("Square", 3);
    waveformSelector.addItem("Triangle", 4);
    addAndMakeVisible(waveformSelector);

    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.parameters, "waveform", waveformSelector);

    audioScope.setBufferSize(128);           // samples shown at once
    audioScope.setSamplesPerBlock(16);       // number of samples pushed each block
    audioScope.setColours(juce::Colours::black, juce::Colours::lime);

    audioProcessor.pushAudioToScope = [this](const juce::AudioBuffer<float>& buffer)
        {
            audioScope.pushBuffer(buffer);
        };

    addAndMakeVisible(audioScope);

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
    audioScope.setBounds(10, 50, getWidth() - 20, getHeight() - 60);
}
