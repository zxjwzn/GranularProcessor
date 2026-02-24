/*
  ==============================================================================
    GlowToggleButton.h
    Toggle button with glow effect for Freeze / Reverse controls.
  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"

class GlowToggleButton : public juce::ToggleButton
{
public:
    GlowToggleButton (const juce::String& text, juce::Colour activeColour = Theme::accentGreen)
        : juce::ToggleButton (text), glowColour (activeColour)
    {
    }

    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                      bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        const bool toggled = getToggleState();

        // Background
        g.setColour (toggled ? glowColour.withAlpha (0.15f) : Theme::panelBackground);
        g.fillRoundedRectangle (bounds, 6.0f);

        // Border
        const float borderAlpha = shouldDrawButtonAsHighlighted ? 1.0f : 0.7f;
        g.setColour ((toggled ? glowColour : Theme::panelBorder).withAlpha (borderAlpha));
        g.drawRoundedRectangle (bounds, 6.0f, 1.5f);

        // Outer glow when active
        if (toggled)
        {
            for (int i = 1; i <= 3; ++i)
            {
                g.setColour (glowColour.withAlpha (0.04f * static_cast<float> (4 - i)));
                g.drawRoundedRectangle (bounds.expanded (static_cast<float> (i) * 1.5f),
                                        6.0f + static_cast<float> (i), 1.0f);
            }
        }

        // Text
        g.setColour (toggled ? glowColour : Theme::textSecondary);
        g.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
        g.drawText (getButtonText(), bounds, juce::Justification::centred);
    }

    void attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& paramID)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, paramID, *this);
    }

private:
    juce::Colour glowColour;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlowToggleButton)
};
