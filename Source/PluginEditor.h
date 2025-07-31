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
#include "CustomLookAndFeel.h"
#include "AnimationDisplayComponent.h"

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

    juce::Slider flutterRateSlider;
    juce::Slider flutterDepthSlider;
    juce::Label flutterRateLabel;
    juce::Label flutterDepthLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flutterDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flutterRateAttachment;

    juce::Slider tremoloDepthSlider, tremoloRateSlider;
    juce::Label tremoloDepthLabel, tremoloRateLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tremoloDepthAttachment, tremoloRateAttachment;



    // === Saw Panel Elements ===
    juce::Slider sawDistortionAmountSlider;
    juce::Label sawDistortionAmountLabel;
    juce::Slider sawDistortionToneSlider;
    juce::Label sawDistortionToneLabel;
    std::unique_ptr < juce::AudioProcessorValueTreeState::SliderAttachment> sawDistortionAmountAttachment;
    std::unique_ptr < juce::AudioProcessorValueTreeState::SliderAttachment> sawDistortionToneAttachment;

    // Saw Bandpass Sweep Effect
    juce::Slider sawSweepRateSlider;
    juce::Label  sawSweepRateLabel;
    juce::Slider sawSweepDepthSlider;
    juce::Label  sawSweepDepthLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sawSweepRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sawSweepDepthAttachment;




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

    CustomLookAndFeel customLookAndFeel;

    AnimationDisplayComponent animationPlaceholder;
    AnimationDisplayComponent polyMalButton;
    AnimationDisplayComponent waveVisualizer;

    BorderedPanel sineFXPanel;
    BorderedPanel sawFXPanel;
    BorderedPanel squareFXPanel;
    BorderedPanel triangleFXPanel;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimalSynthAudioProcessorEditor)
};



