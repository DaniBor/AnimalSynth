#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_core/juce_core.h>

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
