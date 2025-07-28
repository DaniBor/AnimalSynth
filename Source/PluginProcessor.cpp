/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AnimalSynthAudioProcessor::AnimalSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
        BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "PARAMETERS", {
        std::make_unique<juce::AudioParameterChoice>(
            "waveform", "Waveform",
            juce::StringArray { "Sine", "Saw", "Square", "Triangle" },
            0
        ),
        std::make_unique<juce::AudioParameterFloat>("attack",  "Attack",  0.01f, 1.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("decay",   "Decay",   0.01f, 1.0f, 0.2f),
        std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", 0.0f,  1.0f, 0.8f),
        std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.01f, 5.0f, 0.5f)
        })
#endif
{
}

AnimalSynthAudioProcessor::~AnimalSynthAudioProcessor()
{
}

//==============================================================================
const juce::String AnimalSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AnimalSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AnimalSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AnimalSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AnimalSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AnimalSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AnimalSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AnimalSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AnimalSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void AnimalSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AnimalSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    currentSampleRate = sampleRate;
    phase = 0.0;

    currentSampleRate = sampleRate;

    adsrParams.attack = *parameters.getRawParameterValue("attack");
    adsrParams.decay = *parameters.getRawParameterValue("decay");
    adsrParams.sustain = *parameters.getRawParameterValue("sustain");
    adsrParams.release = *parameters.getRawParameterValue("release");
    adsr.setParameters(adsrParams);

    adsr.setSampleRate(currentSampleRate);


    int waveformIndex = *parameters.getRawParameterValue("waveform");
    std::cout << waveformIndex;

    // Choose waveform function ONCE
    switch (static_cast<WaveformType>(waveformIndex))
    {
    case WaveformType::Sine:    currentWaveformFunction = generateSine; break;
    case WaveformType::Saw:     currentWaveformFunction = generateSaw; break;
    case WaveformType::Square:  currentWaveformFunction = generateSquare; break;
    case WaveformType::Triangle:currentWaveformFunction = generateTriangle; break;
    default:                    currentWaveformFunction = generateSine; break;
    }
}

void AnimalSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AnimalSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AnimalSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    int currentWaveformIndex = *parameters.getRawParameterValue("waveform");

    if (currentWaveformIndex != lastWaveformIndex)
    {
        lastWaveformIndex = currentWaveformIndex;

        switch (static_cast<WaveformType>(currentWaveformIndex))
        {
        case WaveformType::Sine:     currentWaveformFunction = generateSine; break;
        case WaveformType::Saw:      currentWaveformFunction = generateSaw; break;
        case WaveformType::Square:   currentWaveformFunction = generateSquare; break;
        case WaveformType::Triangle: currentWaveformFunction = generateTriangle; break;
        default:                     currentWaveformFunction = generateSine; break;
        }
    }



    buffer.clear();

    adsrParams.attack = *parameters.getRawParameterValue("attack");
    adsrParams.decay = *parameters.getRawParameterValue("decay");
    adsrParams.sustain = *parameters.getRawParameterValue("sustain");
    adsrParams.release = *parameters.getRawParameterValue("release");

    adsr.setParameters(adsrParams);


    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            midiNote = msg.getNoteNumber();
            double freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);

            phaseIncrement = freq / currentSampleRate;
            phase = 0.0; // Optional: restart waveform on note-on
            adsr.noteOn();
            noteIsOn = true;
        }
        else if (msg.isNoteOff())
        {
            if (msg.getNoteNumber() == midiNote)
            {
                adsr.noteOff();
                noteIsOn = false;
            }
        }
    }

    auto numChannels = buffer.getNumChannels();
    auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float env = adsr.getNextSample();
        float currentSample = 0.0f;

        if (adsr.isActive())
        {
            currentSample = currentWaveformFunction(phase) * env;
            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;
        }

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.setSample(channel, sample, currentSample);
    }

    /*if (pushAudioToScope)
        pushAudioToScope(buffer);*/

	midiMessages.clear();

}

//==============================================================================
bool AnimalSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AnimalSynthAudioProcessor::createEditor()
{
    return new AnimalSynthAudioProcessorEditor (*this);
}

//==============================================================================
void AnimalSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AnimalSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnimalSynthAudioProcessor();
}


float AnimalSynthAudioProcessor::generateSine(double phase)
{
    return std::sin(juce::MathConstants<double>::twoPi * phase);
}

float AnimalSynthAudioProcessor::generateSaw(double phase)
{
    return static_cast<float>(2.0 * (phase - 0.5));
}

float AnimalSynthAudioProcessor::generateSquare(double phase)
{
    return phase < 0.5 ? 1.0f : -1.0f;
}

float AnimalSynthAudioProcessor::generateTriangle(double phase)
{
    return static_cast<float>(4.0 * std::abs(phase - 0.5) - 1.0);
}