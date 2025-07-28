/*
  ==============================================================================

    BorderedPanel.h
    Created: 28 Jul 2025 1:16:59pm
    Author:  Boro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


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