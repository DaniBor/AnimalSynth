/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AnimationDisplayComponent.h"

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
            // === ADSR Params ===
        std::make_unique<juce::AudioParameterFloat>("attack",  "Attack",  0.01f, 1.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("decay",   "Decay",   0.01f, 1.0f, 0.2f),
        std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", 0.0f,  1.0f, 0.8f),
        std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.01f, 5.0f, 0.5f),

            // === Sine Params ===
        std::make_unique<juce::AudioParameterFloat>("vibratoRate", "Vibrato Rate", 0.0f, 10.0f, 5.0f),
        std::make_unique<juce::AudioParameterFloat>("vibratoDepth", "Vibrato Depth", 0.0f, 0.05f, 0.001f),
        std::make_unique<juce::AudioParameterFloat>("flutterDepth", "Flutter Depth", 0.0f, 0.05f, 0.01f),
        std::make_unique<juce::AudioParameterFloat>("flutterRate",  "Flutter Rate",  0.0f, 20.0f, 5.0f),
        std::make_unique<juce::AudioParameterFloat>("tremoloDepth", "Tremolo Depth", 0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>("tremoloRate",  "Tremolo Rate",  0.0f, 20.0f, 4.0f),

            // === Saw Params ===
        std::make_unique<juce::AudioParameterFloat>(
            "sawDistortionAmount", "Saw Distortion Amount",
            juce::NormalisableRange<float>(1.0f, 10.0f, 0.1f), 3.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "sawDistortionTone", "Saw Distortion Tone",
            juce::NormalisableRange<float>(0.0f, 8000.0f, 1.0f), 5000.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "sawSweepRate", "Sweep Rate",
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f), 2.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "sawSweepDepth", "Sweep Depth",
            juce::NormalisableRange<float>(50.0f, 3000.0f, 1.0f), 1000.0f
        )
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
    vibratoPhase = 0.0;

    adsrParams.attack = *parameters.getRawParameterValue("attack");
    adsrParams.decay = *parameters.getRawParameterValue("decay");
    adsrParams.sustain = *parameters.getRawParameterValue("sustain");
    adsrParams.release = *parameters.getRawParameterValue("release");
    adsr.setParameters(adsrParams);

    adsr.setSampleRate(currentSampleRate);

    flutterAmount.reset(sampleRate, 0.01); // smoothing time
    flutterCounter = 0;
    flutterUpdateInterval = static_cast<int>(sampleRate / *parameters.getRawParameterValue("flutterRate"));


    juce::dsp::ProcessSpec sineSpec;
    sineSpec.sampleRate = currentSampleRate;
    sineSpec.maximumBlockSize = static_cast<juce::uint32> (getBlockSize());
    sineSpec.numChannels = getTotalNumOutputChannels();

    sineFilter.reset();
    sineFilter.prepare(sineSpec);
    sineFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    sineFilter.setCutoffFrequency(1000.0f);
    sineFilter.setResonance(0.8f);

    juce::dsp::ProcessSpec sawSpec;
    sawSpec.sampleRate = sampleRate;
    sawSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    sawSpec.numChannels = getTotalNumOutputChannels();

    sawFilter.prepare(sawSpec);
    sawFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    sawFilter.setResonance(0.9f);
    sawFilter.setCutoffFrequency(1200.0f); // Initial center

    sawDistortion.functionToUse = [](float x) {
        return std::tanh(x * 3.0f); // Soft clipping / saturation
        };

    sawDistortion.prepare({
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        static_cast<juce::uint32>(getTotalNumOutputChannels())
        });

    int waveformIndex = *parameters.getRawParameterValue("waveform");

    AnimalSynthAudioProcessorEditor* e = dynamic_cast<AnimalSynthAudioProcessorEditor*>(getActiveEditor());
    if (e != nullptr) {
        e->animationPlaceholder.setNewAnimal(waveformIndex);
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

    AnimalSynthAudioProcessorEditor* e = dynamic_cast<AnimalSynthAudioProcessorEditor*>(getActiveEditor());
    

    int currentWaveformIndex = *parameters.getRawParameterValue("waveform");

    if (e != nullptr && currentWaveformIndex != e->animationPlaceholder.getIndex()) {
        e->animationPlaceholder.setNewAnimal(currentWaveformIndex);
    }

    buffer.clear();

    adsrParams.attack = *parameters.getRawParameterValue("attack");
    adsrParams.decay = *parameters.getRawParameterValue("decay");
    adsrParams.sustain = *parameters.getRawParameterValue("sustain");
    adsrParams.release = *parameters.getRawParameterValue("release");

    adsr.setParameters(adsrParams);

    switch (static_cast<WaveformType>(currentWaveformIndex))
    {
        case WaveformType::Sine: processSineWave(buffer, midiMessages); break;
        case WaveformType::Saw: processSawWave(buffer, midiMessages); break;
        case WaveformType::Square: processSquareWave(buffer, midiMessages); break;
        case WaveformType::Triangle: processTriangleWave(buffer, midiMessages); break;
        default: buffer.clear(); break;
    }

    if (pushAudioToScope)
        pushAudioToScope(buffer);

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

void AnimalSynthAudioProcessor::processSineWave(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    // Read current vibrato settings
    vibratoRate = *parameters.getRawParameterValue("vibratoRate");
    vibratoDepth = *parameters.getRawParameterValue("vibratoDepth");

    float tremoloRate = *parameters.getRawParameterValue("tremoloRate");
    float tremoloDepth = *parameters.getRawParameterValue("tremoloDepth");

    float cutoff = 300.0f + sinefilterEnvelope * 4000.0f;
    sineFilter.setCutoffFrequency(cutoff);

    // === Handle MIDI ===
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            midiNote = msg.getNoteNumber();
            double freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
            phaseIncrement = freq / currentSampleRate;
            //phase = 0.0;
            //sinefilterEnvelope = 1.0f;
            sineFilterEnvIncrement = 1.0f / (getSampleRate() * 0.25f); // ~250ms decay

            adsr.noteOn();
        }
        else if (msg.isNoteOff())
        {
            if (msg.getNoteNumber() == midiNote)
                adsr.noteOff();
        }
    }

    AnimalSynthAudioProcessorEditor* e = dynamic_cast<AnimalSynthAudioProcessorEditor*>(getActiveEditor());

    // === Synthesis loop ===
    if (adsr.isActive() && e != nullptr) {

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float env = adsr.getNextSample(); // ADSR envelope
            float currentSample = 0.0f;

            
            e->animationPlaceholder.setEnvelopeLevel(env);

            // Vibrato (LFO on frequency)
            float vibrato = std::sin(2.0 * juce::MathConstants<double>::pi * vibratoPhase) * vibratoDepth;
            vibratoPhase += vibratoRate / currentSampleRate;
            if (vibratoPhase >= 1.0)
                vibratoPhase -= 1.0;

            // Flutter update
            if (++flutterCounter >= flutterUpdateInterval)
            {
                flutterCounter = 0;
                float depth = *parameters.getRawParameterValue("flutterDepth");
                float randomValue = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                flutterAmount.setTargetValue(randomValue * depth);
            }

            float flutter = flutterAmount.getNextValue();

            // Phase increment with vibrato and flutter
            double modulatedPhaseInc = phaseIncrement * (1.0 + vibrato + flutter);

            // Amplitude modulation
            float tremolo = 1.0f - (std::sin(2.0f * juce::MathConstants<float>::pi * tremoloPhase) * tremoloDepth);
            tremoloPhase += tremoloRate / currentSampleRate;
            if (tremoloPhase >= 1.0f)
                tremoloPhase -= 1.0f;


            // Sine wave generation
            float rawSine = std::sin(2.0 * juce::MathConstants<double>::pi * phase);
            phase += modulatedPhaseInc;
            if (phase >= 1.0)
                phase -= 1.0;

            // Filter envelope decay
            if (sinefilterEnvelope > 0.0f)
            {
                sinefilterEnvelope -= sineFilterEnvIncrement;
                if (sinefilterEnvelope < 0.0f)
                    sinefilterEnvelope = 0.0f;
            }

            // Modulate filter cutoff frequency
            float cutoff = 300.0f + sinefilterEnvelope * 4000.0f;
            sineFilter.setCutoffFrequency(cutoff);

            // Apply filter to sine wave
            currentSample = sineFilter.processSample(0, rawSine) * env * tremolo;

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.setSample(channel, sample, currentSample);
        }
    }
    else
    {
        if (!adsr.isActive())
            buffer.clear();
    }
}

void AnimalSynthAudioProcessor::processSawWave(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    auto sampleRate = getSampleRate();

    // === Handle MIDI ===
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            midiNote = msg.getNoteNumber();
            double freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);

            phaseIncrement = freq / sampleRate;
            //phase = 0.0;
            adsr.noteOn();

            // Trigger filter envelope
            sawFilterEnvelope = 1.0f;
            sawFilterEnvIncrement = 1.0f / (sampleRate * 0.25f); // 250ms decay
        }
        else if (msg.isNoteOff() && msg.getNoteNumber() == midiNote)
        {
            adsr.noteOff();
        }
    }

    AnimalSynthAudioProcessorEditor* e = dynamic_cast<AnimalSynthAudioProcessorEditor*>(getActiveEditor());

    // === Synthesis loop ===
    if (adsr.isActive() && e != nullptr)
    {
        float toneCutoff = *parameters.getRawParameterValue("sawDistortionTone");
        sawPostFilter.setCutoffFrequency(toneCutoff);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float env = adsr.getNextSample();
            e->animationPlaceholder.setEnvelopeLevel(env);

            // Generate saw wave
            float rawSaw = 2.0f * static_cast<float>(phase) - 1.0f;
            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;

            // Filter envelope decay
            if (sawFilterEnvelope > 0.0f)
            {
                sawFilterEnvelope -= sawFilterEnvIncrement;
                if (sawFilterEnvelope < 0.0f)
                    sawFilterEnvelope = 0.0f;
            }

            // Pre-filter
            float preCutoff = 600.0f + sawFilterEnvelope * 3000.0f;
            sawFilter.setCutoffFrequency(preCutoff);
            float filtered = sawFilter.processSample(0, rawSaw) * env;

            // Distortion
            float distortionAmount = *parameters.getRawParameterValue("sawDistortionAmount");
            float distorted = std::tanh(filtered * distortionAmount);

            // Post-filter (tone shaping)
            float postFiltered = sawPostFilter.processSample(0, distorted);

            // Amplitude modulation for growl
            float am = 1.0f - (std::sin(2.0f * juce::MathConstants<float>::pi * amPhase) * amDepth);
            amPhase += amRate / getSampleRate();
            if (amPhase >= 1.0f)
                amPhase -= 1.0f;

            float finalSample = postFiltered * env * am;

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.setSample(channel, sample, finalSample);
        }
    }
    else
    {
        buffer.clear();
    }

}

void AnimalSynthAudioProcessor::processSquareWave(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    auto sampleRate = getSampleRate();

    // === Handle MIDI ===
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            midiNote = msg.getNoteNumber();
            double freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);

            phaseIncrement = freq / sampleRate;
            //phase = 0.0;
            adsr.noteOn();
        }
        else if (msg.isNoteOff() && msg.getNoteNumber() == midiNote)
        {
            adsr.noteOff();
        }
    }

    // === Synthesis loop ===
    if (adsr.isActive())
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float env = adsr.getNextSample();
            float rawSample = phase < 0.5 ? 1.0f : -1.0f;
            float currentSample = rawSample * env;

            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;

            for (int channel = 0; channel < numChannels; ++channel)
                buffer.setSample(channel, sample, currentSample);
        }
    }
    else
    {
        buffer.clear();
    }
}

void AnimalSynthAudioProcessor::processTriangleWave(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    auto sampleRate = getSampleRate();

    // === Handle MIDI ===
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            midiNote = msg.getNoteNumber();
            double freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);

            phaseIncrement = freq / sampleRate;
            //phase = 0.0;
            adsr.noteOn();
        }
        else if (msg.isNoteOff() && msg.getNoteNumber() == midiNote)
        {
            adsr.noteOff();
        }
    }

    // === Synthesis loop ===
    if (adsr.isActive())
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float env = adsr.getNextSample();
            float rawSample = static_cast<float>(4.0 * std::abs(phase - 0.5) - 1.0);
            float currentSample = rawSample * env;

            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;

            for (int channel = 0; channel < numChannels; ++channel)
                buffer.setSample(channel, sample, currentSample);
        }
    }
    else
    {
        buffer.clear();
    }
}
