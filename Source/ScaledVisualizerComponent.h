#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>


/**
 * @brief A simple Oscilloscope that draws smaller waves than JUCE's builtin one
 */
class ScaledVisualiserComponent : public juce::Component, private juce::Timer
{
public:
    ScaledVisualiserComponent(int bufferSize = 2048)
        : buffer(1, bufferSize)
    {
        buffer.clear();
        startTimerHz(60); // ~60fps
    }


    /**
     * @brief Copies samples from the given buffer into its own to display the Waveforms
     *
     * @param incoming The incoming Samples
     */
    void pushBuffer(const juce::AudioBuffer<float>& incoming)
    {
        const int numSamples = incoming.getNumSamples();
        const float* channelData = incoming.getReadPointer(0); // mono

        const juce::SpinLock::ScopedLockType sl(bufferLock);

        if (numSamples >= buffer.getNumSamples())
        {
            buffer.copyFrom(0, 0, channelData + (numSamples - buffer.getNumSamples()), buffer.getNumSamples());
        }
        else
        {
            buffer.copyFrom(0, 0, buffer.getReadPointer(0, numSamples), buffer.getNumSamples() - numSamples);
            buffer.copyFrom(0, buffer.getNumSamples() - numSamples, channelData, numSamples);
        }
    }

    /**
     *
     * @param zoom The zoom. > 1 to zoom out, < 1 to zoom in
     */
    void setHorizontalZoom(float zoom) // 1.0 = default
    {
        horizontalZoomFactor = zoom;
        repaint();
    }

    /**
     * @brief Takes the Samples in it's buffer and displays them.
     *
     * I didn't like how the builtin simple oscilloscope would cut off the highest and lowest parts.
     * So I made this zoomable one. It iterates over every sample and draws a path between them.
     *
     * @param g The graphic to be displayed.
     */
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::lime);

        const auto bounds = getLocalBounds().toFloat();
        const float midY = bounds.getCentreY();
        const float halfHeight = bounds.getHeight() * 0.5f * 0.8f;

        juce::AudioBuffer<float> localCopy;
        {
            const juce::SpinLock::ScopedLockType sl(bufferLock);
            localCopy.makeCopyOf(buffer);
        }

        const int numSamples = localCopy.getNumSamples();
        const int samplesToRead = juce::jlimit(1, numSamples, static_cast<int>(numSamples / horizontalZoomFactor));
        const int startSample = juce::jmax(0, numSamples - samplesToRead);

        juce::Path path;

        // First sample
        float firstSample = localCopy.getSample(0, startSample);
        float firstY = midY - firstSample * halfHeight;
        path.startNewSubPath(0, firstY);

        for (int x = 1; x < bounds.getWidth(); ++x)
        {
            float normX = static_cast<float>(x) / bounds.getWidth();
            int sampleIndex = startSample + static_cast<int>(normX * samplesToRead);
            sampleIndex = juce::jlimit(0, numSamples - 1, sampleIndex);

            float sample = localCopy.getSample(0, sampleIndex);
            float y = midY - sample * halfHeight;
            path.lineTo((float)x, y);
        }

        g.strokePath(path, juce::PathStrokeType(1.5f));
    }

private:
    void timerCallback() override { repaint(); }

    juce::AudioBuffer<float> buffer;
    juce::SpinLock bufferLock;

    float horizontalZoomFactor = 1.2f;
};
