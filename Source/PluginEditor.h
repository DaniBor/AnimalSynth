/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ScaledVisualizerComponent.h"

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

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AnimalSynthAudioProcessor& audioProcessor;

    juce::ComboBox waveformSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimalSynthAudioProcessorEditor)
};
