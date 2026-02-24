/*
  ==============================================================================
    SectionPanel.h
    Rounded panel with a title header for grouping controls.
  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "CustomLookAndFeel.h"

class SectionPanel : public juce::Component
{
public:
    explicit SectionPanel (const juce::String& title)
        : sectionTitle (title)
    {
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Panel background
        g.setColour (Theme::panelBackground.withAlpha (0.6f));
        g.fillRoundedRectangle (bounds, 8.0f);

        // Panel border
        g.setColour (Theme::panelBorder.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, 1.0f);

        // Title
        auto titleBounds = bounds.removeFromTop (24.0f);
        g.setColour (Theme::textSecondary);
        g.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
        g.drawText (sectionTitle.toUpperCase(), titleBounds.reduced (10.0f, 0.0f),
                    juce::Justification::centredLeft);

        // Title underline
        g.setColour (Theme::panelBorder.withAlpha (0.3f));
        g.drawHorizontalLine (static_cast<int> (titleBounds.getBottom()),
                              bounds.getX() + 8.0f, bounds.getRight() - 8.0f);
    }

    /** Returns the content area below the title */
    juce::Rectangle<int> getContentArea() const
    {
        return getLocalBounds().withTrimmedTop (28).reduced (6, 4);
    }

private:
    juce::String sectionTitle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionPanel)
};
