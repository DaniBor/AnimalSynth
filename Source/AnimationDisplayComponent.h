#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_core/juce_core.h>

/**
 * @brief A simple Component that draws Frames from loaded image based on the ADSR envelope
 *
 * When creating an instance of this Component you need a set of images stored inside a named folder inside the assets folder.
 * You call loadFrames you give it the name of the folder as the parameter.
 *
 * @attention the images should be named like this: <foldername>_<framenumber>
 */
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
