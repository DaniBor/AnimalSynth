#pragma once
#include <juce_gui_basics/juce_gui_basics.h>


class BorderedPanel : public juce::Component
{
public:
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::darkgrey);
        g.drawRoundedRectangle(bounds.reduced(2.0f), 8.0f, 2.0f); // thickness = 2px
    }
};