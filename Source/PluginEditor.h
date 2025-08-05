/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

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

    AnimationDisplayComponent wildlifeCam;

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

    AnimationDisplayComponent logoPanel;
    AnimationDisplayComponent waveVisualizer;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment;

#pragma region PanelElements
    // ===== Sine Panel Elements =====
    juce::Slider vibratoRateSlider, vibratoDepthSlider;
    juce::Label vibratoLabel, vibratoRateLabel, vibratoDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoRateAttachment, vibratoDepthAttachment;

    juce::Slider chorusRateSlider, chorusDepthSlider;
    juce::Label chorusLabel, chorusRateLabel, chorusDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chorusRateAttachment, chorusDepthAttachment;

    juce::Slider tremoloRateSlider, tremoloDepthSlider;
    juce::Label tremoloLabel, tremoloRateLabel, tremoloDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tremoloRateAttachment, tremoloDepthAttachment;


    // ===== Saw Panel Elements =====
    juce::Slider sawCombTimeSlider, sawCombFeedbackSlider;
    juce::Label  sawCombLabel, sawCombTimeLabel, sawCombFeedbackLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sawCombTimeAttachment, sawCombFeedbackAttachment;

    juce::Slider formantFreqSlider, formantResSlider;
    juce::Label formantLabel, formantFreqLabel, formantResLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantFreqAttachment, formantResAttachment;

    juce::Slider sawDriveSlider, sawShapeSlider;
    juce::Label waveshapeLabel, sawDriveLabel, sawShapeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sawDriveAttachment, sawShapeAttachment;





    // ===== Square Panel Elements =====
    juce::Slider squarePunchAmountSlider, squarePunchDecaySlider;
    juce::Label punchLabel, squarePunchAmountLabel, squarePunchDecayLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> squarePunchAmountAttachment, squarePunchDecayAttachment;

    juce::Slider squareBitcrushRateSlider, squareBitcrushDepthSlider;
    juce::Label bitcrushLabel, squareBitcrushRateLabel, squareBitcrushDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> squareBitcrushRateAttachment, squareBitcrushDepthAttachment;

    juce::Slider barkFilterFreqSlider, barkFilterResSlider;
    juce::Label barkFilterLabel, barkFilterFreqLabel, barkFilterResLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> barkFilterFreqAttachment, barkFilterResAttachment;


    // ===== Triangle Panel Elements =====
    juce::Slider triGlideTimeSlider, triGlideDepthSlider;
    juce::Label glideLabel, triGlideTimeLabel, triGlideDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> triGlideTimeAttachment, triGlideDepthAttachment;

    juce::Slider triChirpRateSlider, triChirpDepthSlider;
    juce::Label chirpLabel, triChirpRateLabel, triChirpDepthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> triChirpRateAttachment, triChirpDepthAttachment;

    juce::Slider triEchoTimeSlider, triEchoMixSlider;
    juce::Label echoLabel, triEchoTimeLabel, triEchoMixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> triEchoTimeAttachment, triEchoMixAttachment;
#pragma endregion

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimalSynthAudioProcessorEditor)
};



