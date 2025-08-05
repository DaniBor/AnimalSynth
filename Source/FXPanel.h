#pragma once
#include <juce_gui_basics/juce_gui_basics.h>


/**
 * @brief A basic Component with a background image and border
 */
class FXPanel : public juce::Component
{
public:
    void setImage(juce::Image img)
    {
        backgroundImage = img;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        if (backgroundImage.isValid())
            g.drawImage(backgroundImage, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
        else
            g.fillAll(juce::Colours::darkgrey); // fallback
        
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 2); // 2 pixels thick
    }

protected:
    juce::Image backgroundImage;
};
