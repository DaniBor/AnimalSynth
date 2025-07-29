/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ScaledVisualizerComponent.h"
#include "BorderedPanel.h"

//==============================================================================
/**
*/
class AnimalSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AnimalSynthAudioProcessorEditor (AnimalSynthAudioProcessor&);
    ~AnimalSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    std::unique_ptr<ScaledVisualiserComponent> audioScope;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;


	void updateEffectUI();
    void setupEffectPanels();

    // === Sine Panel Elements ===
    juce::Slider vibratoRateSlider;
    juce::Slider vibratoDepthSlider;
    juce::Label vibratoRateLabel;
    juce::Label vibratoDepthLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoDepthAttachment;

    // === Saw Panel Elements ===
    juce::Slider sawSlider;
    juce::Label  sawLabel;

    // === Square Panel Elements ===
    juce::Slider squareSlider;
    juce::Label  squareLabel;

    // === Triangle Panel Elements ===
    juce::Slider triangleSlider;
    juce::Label  triangleLabel;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AnimalSynthAudioProcessor& audioProcessor;

    juce::ComboBox waveformSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;


    BorderedPanel sineFXPanel;
    BorderedPanel sawFXPanel;
    BorderedPanel squareFXPanel;
    BorderedPanel triangleFXPanel;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimalSynthAudioProcessorEditor)
};



