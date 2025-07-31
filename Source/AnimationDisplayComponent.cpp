#include "AnimationDisplayComponent.h"

AnimationDisplayComponent::AnimationDisplayComponent()
{
    startTimerHz(60); // ~60 FPS for smooth animation
}

void AnimationDisplayComponent::loadFrames(const juce::String& animalName)
{
    frames.clear();
    int index = 0;

    while (true)
    {
        juce::String resourceName = animalName + "_" + juce::String(index) + "_png";

        int dataSize = 0;
        const void* data = BinaryData::getNamedResource(resourceName.toRawUTF8(), dataSize);

        if (data == nullptr)
            break;

        juce::Image img = juce::ImageCache::getFromMemory(data, dataSize);

        if (img.isValid())
            frames.push_back(img);

        ++index;
    }

    repaint();
}

void AnimationDisplayComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::limegreen);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(2.0f), 10.0f, 2.0f);

    if (!frames.empty())
    {
        int index = static_cast<int>(envelopeLevel.load() * static_cast<float>(frames.size() - 1));
        index = juce::jlimit(0, static_cast<int>(frames.size() - 1), index);
        auto& img = frames[(size_t)index];

        g.drawImageWithin(img, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::centred);
    }
    else
    {
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText(txt, getLocalBounds().reduced(10), juce::Justification::centred);
    }
}

void AnimationDisplayComponent::setText(juce::String t)
{
    txt = t;
    repaint();
}

void AnimationDisplayComponent::setEnvelopeLevel(float level)
{
    envelopeLevel.store(level);
}

void AnimationDisplayComponent::setFrames(std::vector<juce::Image> newFrames)
{
    frames = std::move(newFrames);
    repaint();
}

void AnimationDisplayComponent::timerCallback()
{
    repaint();
}
