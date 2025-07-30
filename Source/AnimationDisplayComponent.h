#pragma once
#include <JuceHeader.h>

class AnimationDisplayComponent : public juce::Component
{
    public:
        AnimationDisplayComponent() {
        }

        void paint(juce::Graphics& g) override
        {
            // Background
            g.fillAll(juce::Colours::black);

            // Border
            g.setColour(juce::Colours::limegreen);
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(2.0f), 10.0f, 2.0f);

            // Placeholder text
            g.setColour(juce::Colours::white);
            g.setFont(16.0f);
            g.drawText(txt, getLocalBounds().reduced(10), juce::Justification::centred);
        }

        void setText(juce::String t) {
            txt = t;
        }

    private:
        juce::String txt = "Text.";

};
