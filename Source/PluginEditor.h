/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ScaledVisualizerComponent.h"
#include "FXPanel.h"
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
    


	void updateEffectUI();
    void setupEffectPanels();

    AnimationDisplayComponent animationPlaceholder;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AnimalSynthAudioProcessor& audioProcessor;

    juce::Image backgroundImage;

    juce::Image sineImage, sawImage, squareImage, triangleImage;
    FXPanel sineFXPanel, sawFXPanel, squareFXPanel, triangleFXPanel;

    juce::ComboBox waveformSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

    CustomLookAndFeel customLookAndFeel;

    AnimationDisplayComponent polyMalButton;
    AnimationDisplayComponent waveVisualizer;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;

#pragma region PanelElements
    // ===== Sine Panel Elements =====
    juce::Slider vibratoRateSlider, vibratoDepthSlider;
    juce::Label vibratoRateLabel, vibratoDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoRateAttachment, vibratoDepthAttachment;

    juce::Slider flutterRateSlider, flutterDepthSlider;
    juce::Label flutterRateLabel, flutterDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flutterRateAttachment, flutterDepthAttachment;

    juce::Slider tremoloRateSlider, tremoloDepthSlider;
    juce::Label tremoloRateLabel, tremoloDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tremoloRateAttachment, tremoloDepthAttachment;


    // ===== Saw Panel Elements =====
    juce::Slider sawDistortionAmountSlider, sawDistortionToneSlider;
    juce::Label sawDistortionAmountLabel, sawDistortionToneLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sawDistortionAmountAttachment, sawDistortionToneAttachment;

    juce::Slider sawSweepRateSlider, sawSweepDepthSlider;
    juce::Label  sawSweepRateLabel, sawSweepDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sawSweepRateAttachment, sawSweepDepthAttachment;


    // ===== Square Panel Elements =====
    juce::Slider squarePunchAmountSlider, squarePunchDecaySlider;
    juce::Label squarePunchAmountLabel, squarePunchDecayLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> squarePunchAmountAttachment, squarePunchDecayAttachment;

    juce::Slider squareBitcrushRateSlider, squareBitcrushDepthSlider;
    juce::Label squareBitcrushRateLabel, squareBitcrushDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> squareBitcrushRateAttachment, squareBitcrushDepthAttachment;

    juce::Slider barkFilterFreqSlider, barkFilterResSlider;
    juce::Label barkFilterFreqLabel, barkFilterResLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> barkFilterFreqAttachment, barkFilterResAttachment;


    // ===== Triangle Panel Elements =====
    juce::Slider triGlideTimeSlider, triGlideDepthSlider;
    juce::Label triGlideTimeLabel, triGlideDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> triGlideTimeAttachment, triGlideDepthAttachment;

    juce::Slider triChirpRateSlider, triChirpDepthSlider;
    juce::Label triChirpRateLabel, triChirpDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> triChirpRateAttachment, triChirpDepthAttachment;

    juce::Slider triEchoTimeSlider, triEchoMixSlider;
    juce::Label triEchoTimeLabel, triEchoMixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> triEchoTimeAttachment, triEchoMixAttachment;
#pragma endregion

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimalSynthAudioProcessorEditor)
};



