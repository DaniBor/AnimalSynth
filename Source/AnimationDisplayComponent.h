#pragma once
#include <JuceHeader.h>

class AnimationDisplayComponent : public juce::Component, private juce::Timer
{
public:
    AnimationDisplayComponent();

    void loadFrames(const juce::String& animalName);
    void paint(juce::Graphics& g) override;
    void setText(juce::String t);
    void setEnvelopeLevel(float level);
    void setFrames(std::vector<juce::Image> newFrames);

    void setNewAnimal(int waveformIndex);

    int getIndex();

private:
    void timerCallback() override;

    int curIndex;
    std::vector<juce::Image> frames;
    std::atomic<float> envelopeLevel{ 0.0f };
    juce::String txt = "Text.";
};
