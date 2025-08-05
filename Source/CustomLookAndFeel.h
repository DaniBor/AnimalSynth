#pragma once
#include <juce_gui_basics/juce_gui_basics.h>


/**
 * @brief A class used to give Sliders their custom look
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
    }

    /**
     * @brief draws a simple custom slider
     *
     * @param g the graphic to be drawn
     * @param x x position
     * @param y y posision
     * @param width slider width
     * @param height slider height
     * @param sliderPosProportional
     * @param rotaryStartAngle left end position of the slider
     * @param rotaryEndAngle right end position of the slider
     * @param slider
     */
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                      float sliderPosProportional, float rotaryStartAngle,
                      float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Background circle
        g.setColour(juce::Colours::saddlebrown);
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        // Grey outline
        g.setColour(juce::Colours::darkgrey);
        g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 2.0f);

        // Indicator pointer
        g.setColour(juce::Colours::black);
        juce::Path p;
        float pointerLength = radius * 0.6f;
        float pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.fillPath(p);
    }
};
