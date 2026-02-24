/*
  ==============================================================================
    CustomKnob.h
    Arc-style rotary knob with label and value display.
  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"

class CustomKnob : public juce::Component
{
public:
    CustomKnob (const juce::String& labelText, const juce::String& suffix = "")
        : name (labelText), unitSuffix (suffix)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        slider.setPopupDisplayEnabled (false, false, nullptr);
        addAndMakeVisible (slider);

        nameLabel.setText (labelText, juce::dontSendNotification);
        nameLabel.setJustificationType (juce::Justification::centred);
        nameLabel.setColour (juce::Label::textColourId, Theme::textSecondary);
        nameLabel.setFont (juce::FontOptions (11.0f));
        addAndMakeVisible (nameLabel);

        valueLabel.setJustificationType (juce::Justification::centred);
        valueLabel.setColour (juce::Label::textColourId, Theme::textPrimary);
        valueLabel.setFont (juce::FontOptions (12.0f));
        addAndMakeVisible (valueLabel);

        slider.onValueChange = [this]()
        {
            updateValueLabel();
        };
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        
        const int labelHeight = 16;
        const int valueHeight = 16;

        nameLabel.setBounds (bounds.removeFromTop (labelHeight));
        valueLabel.setBounds (bounds.removeFromBottom (valueHeight));
        slider.setBounds (bounds);
    }

    void attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& paramID)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, paramID, slider);
        updateValueLabel();
    }

    juce::Slider& getSlider() { return slider; }

private:
    void updateValueLabel()
    {
        auto val = slider.getValue();
        juce::String text;

        if (std::abs (val) >= 1000.0)
            text = juce::String (val / 1000.0, 1) + "k";
        else if (std::abs (val) >= 100.0)
            text = juce::String (static_cast<int> (val));
        else if (std::abs (val) >= 10.0)
            text = juce::String (val, 1);
        else
            text = juce::String (val, 2);

        if (unitSuffix.isNotEmpty())
            text += " " + unitSuffix;

        valueLabel.setText (text, juce::dontSendNotification);
    }

    juce::Slider slider;
    juce::Label  nameLabel;
    juce::Label  valueLabel;
    juce::String name;
    juce::String unitSuffix;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomKnob)
};
