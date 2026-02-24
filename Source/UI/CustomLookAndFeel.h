/*
  ==============================================================================
    CustomLookAndFeel.h
    Dark ambient theme for the Granular Processor plugin.
  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

namespace Theme
{
    // Background
    inline const juce::Colour background       { 0xFF0D0D1A };
    inline const juce::Colour panelBackground  { 0xFF14142B };
    inline const juce::Colour panelBorder      { 0xFF2A2A4A };

    // Primary gradient
    inline const juce::Colour primaryCyan      { 0xFF00D4FF };
    inline const juce::Colour primaryPurple    { 0xFF8B5CF6 };

    // Accents
    inline const juce::Colour accentGreen      { 0xFF00FF88 };
    inline const juce::Colour accentPink       { 0xFFFF6EC7 };

    // Text
    inline const juce::Colour textPrimary      { 0xFFE0E0FF };
    inline const juce::Colour textSecondary    { 0xFF6B6B8D };
    inline const juce::Colour textDim          { 0xFF3D3D5C };

    // Knob
    inline const juce::Colour knobBackground   { 0xFF1A1A2E };
    inline const juce::Colour knobTrack        { 0xFF1E1E3A };

    // Toggle
    inline const juce::Colour toggleOff        { 0xFF2A2A4A };
    inline const juce::Colour toggleOn         { 0xFF00FF88 };
}

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Global colours
        setColour (juce::ResizableWindow::backgroundColourId, Theme::background);
        setColour (juce::Label::textColourId, Theme::textPrimary);
        setColour (juce::Slider::textBoxTextColourId, Theme::textPrimary);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ComboBox::backgroundColourId, Theme::panelBackground);
        setColour (juce::ComboBox::textColourId, Theme::textPrimary);
        setColour (juce::ComboBox::outlineColourId, Theme::panelBorder);
        setColour (juce::ComboBox::arrowColourId, Theme::textSecondary);
        setColour (juce::PopupMenu::backgroundColourId, Theme::panelBackground);
        setColour (juce::PopupMenu::textColourId, Theme::textPrimary);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::primaryCyan.withAlpha (0.2f));
        setColour (juce::PopupMenu::highlightedTextColourId, Theme::primaryCyan);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& /*slider*/) override
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        const auto centreX = bounds.getCentreX();
        const auto centreY = bounds.getCentreY();
        const auto rx = centreX - radius;
        const auto ry = centreY - radius;
        const auto rw = radius * 2.0f;
        const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Background circle
        g.setColour (Theme::knobBackground);
        g.fillEllipse (rx, ry, rw, rw);

        // Track arc (full range, dim)
        const float trackThickness = 3.0f;
        juce::Path trackPath;
        trackPath.addCentredArc (centreX, centreY, radius - 4.0f, radius - 4.0f,
                                 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (Theme::knobTrack);
        g.strokePath (trackPath, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));

        // Value arc (gradient from cyan to purple)
        if (sliderPosProportional > 0.0f)
        {
            juce::Path valuePath;
            valuePath.addCentredArc (centreX, centreY, radius - 4.0f, radius - 4.0f,
                                     0.0f, rotaryStartAngle, angle, true);

            juce::ColourGradient gradient (Theme::primaryCyan, centreX - radius, centreY,
                                           Theme::primaryPurple, centreX + radius, centreY, false);
            g.setGradientFill (gradient);
            g.strokePath (valuePath, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                                            juce::PathStrokeType::rounded));

            // Glow effect
            auto glowPath = valuePath;
            g.setColour (Theme::primaryCyan.withAlpha (0.15f));
            g.strokePath (glowPath, juce::PathStrokeType (trackThickness + 4.0f, juce::PathStrokeType::curved,
                                                            juce::PathStrokeType::rounded));
        }

        // Pointer dot
        const float pointerLength = radius - 10.0f;
        const float pointerX = centreX + pointerLength * std::cos (angle - juce::MathConstants<float>::halfPi);
        const float pointerY = centreY + pointerLength * std::sin (angle - juce::MathConstants<float>::halfPi);

        g.setColour (Theme::textPrimary);
        g.fillEllipse (pointerX - 3.0f, pointerY - 3.0f, 6.0f, 6.0f);

        // Inner circle highlight
        const float innerRadius = radius * 0.35f;
        juce::ColourGradient innerGradient (Theme::knobBackground.brighter (0.1f),
                                             centreX, centreY - innerRadius * 0.5f,
                                             Theme::knobBackground, centreX, centreY + innerRadius,
                                             true);
        g.setGradientFill (innerGradient);
        g.fillEllipse (centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                       int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                       juce::ComboBox& /*box*/) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
        g.setColour (Theme::panelBackground);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (Theme::panelBorder);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        // Arrow
        auto arrowBounds = juce::Rectangle<int> (width - 24, 0, 20, height).toFloat();
        juce::Path arrow;
        auto arrowCentre = arrowBounds.getCentre();
        arrow.addTriangle (arrowCentre.x - 4.0f, arrowCentre.y - 2.0f,
                          arrowCentre.x + 4.0f, arrowCentre.y - 2.0f,
                          arrowCentre.x, arrowCentre.y + 3.0f);
        g.setColour (Theme::textSecondary);
        g.fillPath (arrow);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool /*shouldDrawButtonAsHighlighted*/,
                           bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
        const bool isOn = button.getToggleState();

        // Background
        g.setColour (isOn ? Theme::accentGreen.withAlpha (0.15f) : Theme::panelBackground);
        g.fillRoundedRectangle (bounds, 6.0f);

        // Border
        g.setColour (isOn ? Theme::accentGreen : Theme::panelBorder);
        g.drawRoundedRectangle (bounds, 6.0f, 1.5f);

        // Glow when on
        if (isOn)
        {
            g.setColour (Theme::accentGreen.withAlpha (0.08f));
            g.fillRoundedRectangle (bounds.expanded (2.0f), 8.0f);
        }

        // Text
        g.setColour (isOn ? Theme::accentGreen : Theme::textSecondary);
        g.setFont (juce::FontOptions (13.0f));
        g.drawText (button.getButtonText(), bounds, juce::Justification::centred);
    }

    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        g.setColour (label.findColour (juce::Label::textColourId));
        g.setFont (label.getFont());
        g.drawText (label.getText(), label.getLocalBounds(), label.getJustificationType(), false);
    }

    juce::Font getComboBoxFont (juce::ComboBox& /*box*/) override
    {
        return juce::Font (juce::FontOptions (13.0f));
    }

    juce::Font getPopupMenuFont() override
    {
        return juce::Font (juce::FontOptions (13.0f));
    }
};
