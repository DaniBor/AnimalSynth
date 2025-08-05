/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AnimationDisplayComponent.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>



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
        std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.01f, 3.0f, 0.5f),

            // === Sine Params ===
        std::make_unique<juce::AudioParameterFloat>("vibratoRate", "Vibrato Rate", 0.0f, 10.0f, 5.0f),
        std::make_unique<juce::AudioParameterFloat>("vibratoDepth", "Vibrato Depth", 0.0f, 0.05f, 0.001f),
        std::make_unique<juce::AudioParameterFloat>("sineChorusRate", "Chorus Rate", 0.0f, 10.0f, 1.5f),
        std::make_unique<juce::AudioParameterFloat>("sineChorusDepth", "Chorus Depth", 0.0f, 1.0f, 0.3f),
        std::make_unique<juce::AudioParameterFloat>("tremoloDepth", "Tremolo Depth", 0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>("tremoloRate",  "Tremolo Rate",  0.0f, 20.0f, 4.0f),

            // === Saw Params ===
        std::make_unique<juce::AudioParameterFloat>(
            "sawCombTime", "Comb Delay Time",
            juce::NormalisableRange<float>(1.0f, 30.0f, 0.1f), 10.0f // milliseconds
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "sawCombFeedback", "Comb Feedback",
            juce::NormalisableRange<float>(0.0f, 0.70f, 0.01f), 0.25f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "formantFreq", "Formant Frequency",
            juce::NormalisableRange<float>(200.0f, 2000.0f, 1.0f), 800.0f
            ),
        std::make_unique<juce::AudioParameterFloat>(
            "formantResonance", "Formant Resonance",
            juce::NormalisableRange<float>(0.0f, 2.5f, 0.01f), 1.0f
            ),
        std::make_unique<juce::AudioParameterFloat>(
            "sawDrive", "Drive",
            juce::NormalisableRange<float>(0.9f, 10.0f, 0.1f), 3.0f
            ),
        std::make_unique<juce::AudioParameterFloat>(
            "sawShape", "Shape",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f
            ),

            // === Square Params ===
        std::make_unique<juce::AudioParameterFloat>(
            "squarePunchAmount", "Punch Amount",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f
        ),

        std::make_unique<juce::AudioParameterFloat>(
            "squarePunchDecay", "Punch Decay",
            juce::NormalisableRange<float>(0.01f, 0.3f, 0.01f), 0.05f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "squareBitcrushRate", "Bitcrush Rate",
            juce::NormalisableRange<float>(100.0f, 8000.0f, 1.0f), 10000.0f // Hz
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "squareBitcrushDepth", "Bitcrush Depth",
            juce::NormalisableRange<float>(1.0f, 16.0f, 1.0f), 16.0f // Bits
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "barkFilterFreq", "Bark Freq",
            juce::NormalisableRange<float>(300.0f, 3000.0f, 1.0f), 800.0f
        ),

        std::make_unique<juce::AudioParameterFloat>(
            "barkFilterResonance", "Bark Res",
            juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f), 1.0f
        ),

            // === Triangle Params ===
        std::make_unique<juce::AudioParameterFloat>("triGlideTime", "Glide Time", 0.0f, 0.2f, 0.05f),
        std::make_unique<juce::AudioParameterFloat>(
            "triGlideDepth", "Glide Depth",
            juce::NormalisableRange<float>(1.0f, 24.0f, 1.0f), // Range: 1 to 24 semitones, step of 1
            12.0f // default value
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "triChirpRate", "Chirp Rate",
            juce::NormalisableRange<float>(1.0f, 50.0f, 0.1f), 20.0f // Hz
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "triChirpDepth", "Chirp Depth",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f // 0 to full modulation
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "triEchoTime", "Echo Time",
            juce::NormalisableRange<float>(10.0f, 250.0f, 1.0f), 80.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "triEchoMix", "Echo Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f
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

    int waveformIndex = *parameters.getRawParameterValue("waveform");

    adsrParams.attack = *parameters.getRawParameterValue("attack");
    adsrParams.decay = *parameters.getRawParameterValue("decay");
    adsrParams.sustain = *parameters.getRawParameterValue("sustain");
    adsrParams.release = *parameters.getRawParameterValue("release");
    adsr.setParameters(adsrParams);

    adsr.setSampleRate(currentSampleRate);

    // ====== Prepare Sine ======
    juce::dsp::ProcessSpec chrousSpec;
    chrousSpec.sampleRate = sampleRate;
    chrousSpec.maximumBlockSize = samplesPerBlock;
    chrousSpec.numChannels = getTotalNumOutputChannels();

    sineChorus.prepare(chrousSpec);
    sineChorus.setMix(0.5f); // 50% wet/dry mix


    juce::dsp::ProcessSpec sineSpec;
    sineSpec.sampleRate = currentSampleRate;
    sineSpec.maximumBlockSize = static_cast<juce::uint32> (getBlockSize());
    sineSpec.numChannels = getTotalNumOutputChannels();

    sineFilter.reset();
    sineFilter.prepare(sineSpec);
    sineFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    sineFilter.setCutoffFrequency(1000.0f);
    sineFilter.setResonance(0.8f);

    // ====== Prepare Saw ======
    sawCombBuffer.setSize(getTotalNumOutputChannels(), static_cast<int>(sampleRate * 0.05)); // 50ms max
    sawCombBuffer.clear();
    sawCombWritePosition = 0;



    juce::dsp::ProcessSpec formantSpec;
    formantSpec.sampleRate = sampleRate;
    formantSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    formantSpec.numChannels = getTotalNumOutputChannels();

    formantFilter.prepare(formantSpec);
    formantFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);


    // ====== Prepare Square ======
    juce::dsp::ProcessSpec barkSpec{
    sampleRate,
    static_cast<juce::uint32>(samplesPerBlock),
    static_cast<juce::uint32>(getTotalNumOutputChannels())
    };

    barkFilter.reset();
    barkFilter.prepare(barkSpec);
    barkFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    barkFilter.setCutoffFrequency(800.0f);  // Default
    barkFilter.setResonance(1.0f);


    // ====== Prepare Triangle ======
    echoBuffer.setSize(getTotalNumOutputChannels(), (int)(getSampleRate() * 2)); // 500 ms max
    echoBuffer.clear();
    echoWritePosition = 0;

    auto* e = dynamic_cast<AnimalSynthAudioProcessorEditor*>(getActiveEditor());
    if (e != nullptr) {
        e->wildlifeCam.setNewAnimal(waveformIndex);
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

    if (e != nullptr && currentWaveformIndex != e->wildlifeCam.getIndex()) {
        e->wildlifeCam.setNewAnimal(currentWaveformIndex);
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
// This creates new instances of the plugin.
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

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    sineChorus.setRate(*parameters.getRawParameterValue("sineChorusRate"));
    sineChorus.setDepth(*parameters.getRawParameterValue("sineChorusDepth"));

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
            float env = adsr.getNextSample();
            float currentSample = 0.0f;

            e->wildlifeCam.setEnvelopeLevel(env);

            // Vibrato
            float vibrato = std::sin(2.0 * juce::MathConstants<double>::pi * vibratoPhase) * vibratoDepth;
            vibratoPhase += vibratoRate / currentSampleRate;
            if (vibratoPhase >= 1.0)
                vibratoPhase -= 1.0;

            double modulatedPhaseInc = phaseIncrement * (1.0 + vibrato);

            // Tremolo
            float tremolo = 1.0f - (std::sin(2.0f * juce::MathConstants<float>::pi * tremoloPhase) * tremoloDepth);
            tremoloPhase += tremoloRate / currentSampleRate;
            if (tremoloPhase >= 1.0f)
                tremoloPhase -= 1.0f;

            // Sine generation
            float rawSine = std::sin(2.0 * juce::MathConstants<double>::pi * phase);
            phase += modulatedPhaseInc;
            if (phase >= 1.0)
                phase -= 1.0;

            // Filter envelope
            if (sinefilterEnvelope > 0.0f)
            {
                sinefilterEnvelope -= sineFilterEnvIncrement;
                if (sinefilterEnvelope < 0.0f)
                    sinefilterEnvelope = 0.0f;
            }

            float cutoff = 300.0f + sinefilterEnvelope * 4000.0f;
            sineFilter.setCutoffFrequency(cutoff);
            float filtered = sineFilter.processSample(0, rawSine);

            currentSample = filtered * env * tremolo;

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.setSample(channel, sample, currentSample);
        }
        sineChorus.setCentreDelay(10.0f);  // Default value, tweak if needed
        sineChorus.setFeedback(0.0f);      // Optional
        sineChorus.setMix(0.4f);           // Wet/dry balance

        sineChorus.process(context);
    }
    else
    {
        if (!adsr.isActive())
            buffer.clear();
    }
}

void AnimalSynthAudioProcessor::processSawWave(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const double sampleRate = getSampleRate();

    float combDelayMs = *parameters.getRawParameterValue("sawCombTime");
    float combFeedback = *parameters.getRawParameterValue("sawCombFeedback");

    float formantFreq = *parameters.getRawParameterValue("formantFreq");
    float formantRes = *parameters.getRawParameterValue("formantResonance");

    float drive = *parameters.getRawParameterValue("sawDrive");
    float shape = *parameters.getRawParameterValue("sawShape");

    int maxDelaySamples = sawCombBuffer.getNumSamples();
    int delaySamples = static_cast<int>((combDelayMs / 1000.0f) * sampleRate);
    delaySamples = std::clamp(delaySamples, 1, maxDelaySamples - 1);

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            midiNote = msg.getNoteNumber();
            double freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
            phaseIncrement = freq / sampleRate;
            adsr.noteOn();
        }
        else if (msg.isNoteOff() && msg.getNoteNumber() == midiNote)
        {
            adsr.noteOff();
        }
    }

    if (adsr.isActive())
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float env = adsr.getNextSample();

            float rawSaw = 2.0f * static_cast<float>(phase) - 1.0f;
            float shaped = rawSaw * env;

            // === Formant Filter ===
            if (formantRes > 0.0f)
            {
                formantFilter.setCutoffFrequency(formantFreq);
                formantFilter.setResonance(formantRes);

                float filtered = formantFilter.processSample(0, shaped);
                shaped = filtered * env;
            }

            // === Waveshaping ===
            float waveshaped = shaped;

            if (drive > 0.9f)
            {
                float driven = shaped * drive;
                float hard = juce::jlimit(-1.0f, 1.0f, driven);
                float soft = std::tanh(driven);
                waveshaped = juce::jmap(shape, hard, soft);
            }

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* delayData = sawCombBuffer.getWritePointer(ch);
                float* output = buffer.getWritePointer(ch);

                int readPos = (sawCombWritePosition + maxDelaySamples - delaySamples) % maxDelaySamples;
                float delayed = delayData[readPos];

                float processed = waveshaped + delayed * combFeedback;

                output[sample] = processed;
                delayData[sawCombWritePosition] = processed;
            }

            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;

            sawCombWritePosition = (sawCombWritePosition + 1) % maxDelaySamples;
        }
    }
    else
    {
        buffer.clear();
        sawCombBuffer.clear();
        sawCombWritePosition = 0;
    }
}






void AnimalSynthAudioProcessor::processSquareWave(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    auto sampleRate = getSampleRate();

    float crushRate = *parameters.getRawParameterValue("squareBitcrushRate");
    float crushDepth = *parameters.getRawParameterValue("squareBitcrushDepth");

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

            squarePunchLevel = 1.0f;

            // Start the bark envelope only on note-on
            barkFilterEnvelope = 1.0f;
            barkFilterDecayRate = 1.0f / (sampleRate * 0.15f);
        }
    }

    AnimalSynthAudioProcessorEditor* e = dynamic_cast<AnimalSynthAudioProcessorEditor*>(getActiveEditor());

    // === Synthesis loop ===
    if (adsr.isActive() && e != nullptr)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float env = adsr.getNextSample();
            e->wildlifeCam.setEnvelopeLevel(env);
            float rawSample = (phase < 0.5f) ? 1.0f : -1.0f;

            // --- Punch Envelope ---
            if (squarePunchLevel > 0.0f)
            {
                squarePunchLevel -= squarePunchDecayRate;
                if (squarePunchLevel < 0.0f)
                    squarePunchLevel = 0.0f;
            }
            float punchEnv = 1.0f + squarePunchLevel;

            float currentSample = rawSample * env * punchEnv;

            // --- Bitcrusher ---
            bool bitcrusherActive = crushDepth > 1.0f;
            if (bitcrusherActive)
            {
                int bitDepth = static_cast<int>(std::round(crushDepth));
                bitDepth = std::clamp(bitDepth, 1, 16); // Prevent extreme values

                int quantizationLevels = (1 << bitDepth) - 1;

                int samplesPerHold = std::max(1, static_cast<int>(sampleRate / crushRate));

                if (bitcrushCounter == 0)
                {
                    // Quantize current sample
                    lastBitcrushedSample = std::round(currentSample * quantizationLevels) / quantizationLevels;
                }

                currentSample = lastBitcrushedSample;

                bitcrushCounter = (bitcrushCounter + 1) % samplesPerHold;
            }


            // Bark filter envelope decay
            if (barkFilterEnvelope > 0.0f)
            {
                barkFilterEnvelope -= barkFilterDecayRate;
                if (barkFilterEnvelope < 0.0f)
                    barkFilterEnvelope = 0.0f;
            }

            // Set dynamic bandpass cutoff and resonance
            float baseFreq = *parameters.getRawParameterValue("barkFilterFreq");
            float res = *parameters.getRawParameterValue("barkFilterResonance");
            float modulatedCutoff = baseFreq + barkFilterEnvelope * 2000.0f; // Sweep range

            barkFilter.setCutoffFrequency(modulatedCutoff);
            barkFilter.setResonance(res);

            // Apply to sample
            currentSample = barkFilter.processSample(0, currentSample);


            // --- Phase update ---
            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;

            // --- Output to buffer ---
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
    const int numSamples   = buffer.getNumSamples();
    const int numChannels  = buffer.getNumChannels();
    const float sampleRate = getSampleRate();

    float chirpRate  = *parameters.getRawParameterValue("triChirpRate");
    float chirpDepth = *parameters.getRawParameterValue("triChirpDepth");

    float echoTimeMs = *parameters.getRawParameterValue("triEchoTime");
    float echoMix    = *parameters.getRawParameterValue("triEchoMix");

    const int delaySamples     = static_cast<int>((echoTimeMs / 1000.0f) * sampleRate);
    const int echoBufferLength = echoBuffer.getNumSamples();

    // === MIDI handling ===
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            midiNote = msg.getNoteNumber();

            double targetFreq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
            float glideDepth  = *parameters.getRawParameterValue("triGlideDepth");
            float glideTime   = *parameters.getRawParameterValue("triGlideTime");

            double startFreq = targetFreq * std::pow(2.0, -glideDepth / 12.0);

            glideStartFreq     = startFreq;
            glideTargetFreq    = targetFreq;
            glideCurrentFreq   = startFreq;
            glideSamplesLeft   = static_cast<int>(glideTime * sampleRate);
            glideStep          = (glideSamplesLeft > 0) ? (glideTargetFreq - glideStartFreq) / glideSamplesLeft : 0.0;

            adsr.noteOn();
        }
        else if (msg.isNoteOff() && msg.getNoteNumber() == midiNote)
        {
            adsr.noteOff();
        }
    }

    auto* e = dynamic_cast<AnimalSynthAudioProcessorEditor*>(getActiveEditor());

    if (!adsr.isActive())
    {
        buffer.clear();
        echoBuffer.clear();
        echoWritePosition = 0;
        return;
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float env = adsr.getNextSample();
        if (e != nullptr) e->wildlifeCam.setEnvelopeLevel(env);

        // === Glide Update ===
        if (glideSamplesLeft > 0)
        {
            glideCurrentFreq += glideStep;
            --glideSamplesLeft;
        }
        else
        {
            glideCurrentFreq = glideTargetFreq;
        }

        // === Triangle oscillator ===
        phaseIncrement = glideCurrentFreq / sampleRate;
        float rawSample = static_cast<float>(4.0 * std::abs(phase - 0.5) - 1.0);

        // === Chirp (AM) ===
        float am = 1.0f - (std::sin(2.0f * juce::MathConstants<float>::pi * chirpPhase) * chirpDepth);
        chirpPhase += chirpRate / sampleRate;
        if (chirpPhase >= 1.0f) chirpPhase -= 1.0f;

        float drySample = rawSample * env * am;

        // === Echo with fade-out based on ADSR ===
        float echoFade = juce::jlimit(0.0f, 1.0f, env); // 0 when envelope is silent, 1 at peak

        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* echoData = echoBuffer.getWritePointer(channel);
            int readPos     = (echoWritePosition + echoBufferLength - delaySamples) % echoBufferLength;

            float delayedSample = echoData[readPos] * echoFade;
            float wetSample     = (1.0f - echoMix) * drySample + echoMix * delayedSample;

            float feedback = delayedSample * 0.4f * env;

            echoData[echoWritePosition] = drySample + feedback;

            buffer.setSample(channel, sample, wetSample);
        }

        phase += phaseIncrement;
        if (phase >= 1.0) phase -= 1.0;

        echoWritePosition = (echoWritePosition + 1) % echoBufferLength;
    }
}