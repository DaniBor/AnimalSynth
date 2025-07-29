/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class AnimalSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AnimalSynthAudioProcessor();
    ~AnimalSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    std::function<void(const juce::AudioBuffer<float>&)> pushAudioToScope;

    juce::AudioProcessorValueTreeState parameters;

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;


private:
    //=============================================================================
    enum class WaveformType
    {
        Sine,
        Saw,
        Square,
        Triangle
    };

    //WaveformType waveformType = WaveformType::Square;
    using WaveformFunction = float(*)(double);
    WaveformFunction currentWaveformFunction = nullptr;

    double currentSampleRate = 44100.0;
    double phase = 0.0;
    double phaseIncrement = 0.0;
    int midiNote = -1;
    bool noteIsOn = false;

    void processSineWave(juce::AudioBuffer<float>&, juce::MidiBuffer&);
    void processSawWave(juce::AudioBuffer<float>&, juce::MidiBuffer&);
    void processSquareWave(juce::AudioBuffer<float>&, juce::MidiBuffer&);
    void processTriangleWave(juce::AudioBuffer<float>&, juce::MidiBuffer&);


    // === Sine Filter and FX ===
    juce::dsp::StateVariableTPTFilter<float> sineFilter;
    float sinefilterEnvelope = 0.0f;
    float sineFilterEnvIncrement = 0.0f;

    double vibratoPhase = 0.0;
    double vibratoRate = 5.0;
    double vibratoDepth = 0.005;

    // === Saw Filter and FX ===

    // === Square Filter and FX ===

    // === Triangle Filter and FX ===

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimalSynthAudioProcessor)
};
