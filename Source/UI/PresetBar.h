/*
  ==============================================================================
    PresetBar.h
    Top bar with preset selection, navigation, and save functionality.
  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "CustomLookAndFeel.h"
#include "../Utils/ParamIDs.h"

#include <map>
#include <vector>

class PresetBar : public juce::Component
{
public:
    PresetBar (juce::AudioProcessorValueTreeState& apvts)
        : valueTreeState (apvts)
    {
        buildPresets();

        // Title
        titleLabel.setText ("GRANULAR", juce::dontSendNotification);
        titleLabel.setFont (juce::FontOptions (16.0f).withStyle ("Bold"));
        titleLabel.setColour (juce::Label::textColourId, Theme::primaryCyan);
        addAndMakeVisible (titleLabel);

        // Preset combo
        for (int i = 0; i < static_cast<int> (presets.size()); ++i)
            presetCombo.addItem (presets[static_cast<size_t> (i)].name, i + 1);

        presetCombo.setSelectedId (1, juce::dontSendNotification);
        presetCombo.onChange = [this]() { loadSelectedPreset(); };
        addAndMakeVisible (presetCombo);

        // Navigation buttons
        prevButton.setButtonText ("<");
        prevButton.onClick = [this]() {
            int current = presetCombo.getSelectedId();
            if (current > 1) presetCombo.setSelectedId (current - 1);
        };
        addAndMakeVisible (prevButton);

        nextButton.setButtonText (">");
        nextButton.onClick = [this]() {
            int current = presetCombo.getSelectedId();
            if (current < presetCombo.getNumItems()) presetCombo.setSelectedId (current + 1);
        };
        addAndMakeVisible (nextButton);

        // Save button
        saveButton.setButtonText ("Save");
        saveButton.onClick = [this]() { savePreset(); };
        addAndMakeVisible (saveButton);

        // Style buttons
        for (auto* btn : { &prevButton, &nextButton, &saveButton })
        {
            btn->setColour (juce::TextButton::buttonColourId, Theme::panelBackground);
            btn->setColour (juce::TextButton::textColourOffId, Theme::textSecondary);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8, 4);

        titleLabel.setBounds (bounds.removeFromLeft (120));

        saveButton.setBounds (bounds.removeFromRight (60));
        bounds.removeFromRight (4);
        nextButton.setBounds (bounds.removeFromRight (30));
        bounds.removeFromRight (2);
        prevButton.setBounds (bounds.removeFromRight (30));
        bounds.removeFromRight (8);

        presetCombo.setBounds (bounds);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (Theme::panelBackground.withAlpha (0.8f));
        g.fillRect (bounds);

        // Bottom line
        g.setColour (Theme::panelBorder.withAlpha (0.5f));
        g.drawHorizontalLine (getHeight() - 1, 0.0f, static_cast<float> (getWidth()));
    }

private:
    struct Preset
    {
        juce::String name;
        std::map<juce::String, float> values;
    };

    void loadSelectedPreset()
    {
        const int idx = presetCombo.getSelectedId() - 1;
        if (idx < 0 || idx >= static_cast<int> (presets.size()))
            return;

        const auto& preset = presets[static_cast<size_t> (idx)];

        for (const auto& [paramID, value] : preset.values)
        {
            if (auto* param = valueTreeState.getParameter (paramID))
            {
                param->setValueNotifyingHost (
                    param->convertTo0to1 (value));
            }
        }
    }

    void savePreset()
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon,
                                                 "Save Preset", "Preset saving coming soon!");
    }

    void buildPresets()
    {
        // --- 1) Init ---
        presets.push_back ({ "Init", {
            { ParamIDs::grainSize,     100.0f },
            { ParamIDs::grainDensity,  8.0f },
            { ParamIDs::grainPosition, 50.0f },
            { ParamIDs::grainPitch,    0.0f },
            { ParamIDs::grainPan,      0.0f },
            { ParamIDs::posScatter,    20.0f },
            { ParamIDs::pitchScatter,  0.0f },
            { ParamIDs::panScatter,    30.0f },
            { ParamIDs::grainAttack,   25.0f },
            { ParamIDs::grainDecay,    25.0f },
            { ParamIDs::envelopeShape, 0.0f },
            { ParamIDs::freeze,        0.0f },
            { ParamIDs::reverse,       0.0f },
            { ParamIDs::feedback,      0.0f },
            { ParamIDs::shimmer,       0.0f },
            { ParamIDs::lowCut,        20.0f },
            { ParamIDs::highCut,       20000.0f },
            { ParamIDs::lfoRate,       1.0f },
            { ParamIDs::lfoDepth,      0.0f },
            { ParamIDs::lfoShape,      0.0f },
            { ParamIDs::lfoTarget,     1.0f },
            { ParamIDs::stereoWidth,   100.0f },
            { ParamIDs::outputLevel,   0.0f },
            { ParamIDs::dryWet,        50.0f },
            { ParamIDs::bufferLength,  4.0f },
        }});

        // --- 2) Ambient Pad ---
        presets.push_back ({ "Ambient Pad", {
            { ParamIDs::grainSize,     250.0f },
            { ParamIDs::grainDensity,  12.0f },
            { ParamIDs::grainPosition, 50.0f },
            { ParamIDs::grainPitch,    0.0f },
            { ParamIDs::grainPan,      0.0f },
            { ParamIDs::posScatter,    40.0f },
            { ParamIDs::pitchScatter,  5.0f },
            { ParamIDs::panScatter,    60.0f },
            { ParamIDs::grainAttack,   40.0f },
            { ParamIDs::grainDecay,    40.0f },
            { ParamIDs::envelopeShape, 0.0f },
            { ParamIDs::freeze,        0.0f },
            { ParamIDs::reverse,       0.0f },
            { ParamIDs::feedback,      0.25f },
            { ParamIDs::shimmer,       0.0f },
            { ParamIDs::lowCut,        80.0f },
            { ParamIDs::highCut,       12000.0f },
            { ParamIDs::lfoRate,       0.2f },
            { ParamIDs::lfoDepth,      30.0f },
            { ParamIDs::lfoShape,      0.0f },
            { ParamIDs::lfoTarget,     1.0f },
            { ParamIDs::stereoWidth,   150.0f },
            { ParamIDs::outputLevel,   0.0f },
            { ParamIDs::dryWet,        70.0f },
            { ParamIDs::bufferLength,  6.0f },
        }});

        // --- 3) Frozen Texture ---
        presets.push_back ({ "Frozen Texture", {
            { ParamIDs::grainSize,     300.0f },
            { ParamIDs::grainDensity,  15.0f },
            { ParamIDs::grainPosition, 50.0f },
            { ParamIDs::grainPitch,    0.0f },
            { ParamIDs::grainPan,      0.0f },
            { ParamIDs::posScatter,    60.0f },
            { ParamIDs::pitchScatter,  8.0f },
            { ParamIDs::panScatter,    80.0f },
            { ParamIDs::grainAttack,   35.0f },
            { ParamIDs::grainDecay,    35.0f },
            { ParamIDs::envelopeShape, 1.0f },
            { ParamIDs::freeze,        1.0f },
            { ParamIDs::reverse,       0.0f },
            { ParamIDs::feedback,      0.4f },
            { ParamIDs::shimmer,       0.0f },
            { ParamIDs::lowCut,        100.0f },
            { ParamIDs::highCut,       10000.0f },
            { ParamIDs::lfoRate,       0.1f },
            { ParamIDs::lfoDepth,      40.0f },
            { ParamIDs::lfoShape,      0.0f },
            { ParamIDs::lfoTarget,     1.0f },
            { ParamIDs::stereoWidth,   160.0f },
            { ParamIDs::outputLevel,   0.0f },
            { ParamIDs::dryWet,        85.0f },
            { ParamIDs::bufferLength,  8.0f },
        }});

        // --- 4) Shimmer Cloud ---
        presets.push_back ({ "Shimmer Cloud", {
            { ParamIDs::grainSize,     200.0f },
            { ParamIDs::grainDensity,  10.0f },
            { ParamIDs::grainPosition, 50.0f },
            { ParamIDs::grainPitch,    12.0f },
            { ParamIDs::grainPan,      0.0f },
            { ParamIDs::posScatter,    35.0f },
            { ParamIDs::pitchScatter,  10.0f },
            { ParamIDs::panScatter,    70.0f },
            { ParamIDs::grainAttack,   30.0f },
            { ParamIDs::grainDecay,    45.0f },
            { ParamIDs::envelopeShape, 0.0f },
            { ParamIDs::freeze,        0.0f },
            { ParamIDs::reverse,       0.0f },
            { ParamIDs::feedback,      0.3f },
            { ParamIDs::shimmer,       65.0f },
            { ParamIDs::lowCut,        150.0f },
            { ParamIDs::highCut,       16000.0f },
            { ParamIDs::lfoRate,       0.15f },
            { ParamIDs::lfoDepth,      20.0f },
            { ParamIDs::lfoShape,      0.0f },
            { ParamIDs::lfoTarget,     2.0f },
            { ParamIDs::stereoWidth,   180.0f },
            { ParamIDs::outputLevel,   -3.0f },
            { ParamIDs::dryWet,        75.0f },
            { ParamIDs::bufferLength,  5.0f },
        }});

        // --- 5) Glitch Scatter ---
        presets.push_back ({ "Glitch Scatter", {
            { ParamIDs::grainSize,     30.0f },
            { ParamIDs::grainDensity,  35.0f },
            { ParamIDs::grainPosition, 50.0f },
            { ParamIDs::grainPitch,    0.0f },
            { ParamIDs::grainPan,      0.0f },
            { ParamIDs::posScatter,    90.0f },
            { ParamIDs::pitchScatter,  60.0f },
            { ParamIDs::panScatter,    100.0f },
            { ParamIDs::grainAttack,   5.0f },
            { ParamIDs::grainDecay,    10.0f },
            { ParamIDs::envelopeShape, 3.0f },
            { ParamIDs::freeze,        0.0f },
            { ParamIDs::reverse,       0.0f },
            { ParamIDs::feedback,      0.15f },
            { ParamIDs::shimmer,       0.0f },
            { ParamIDs::lowCut,        200.0f },
            { ParamIDs::highCut,       18000.0f },
            { ParamIDs::lfoRate,       8.0f },
            { ParamIDs::lfoDepth,      50.0f },
            { ParamIDs::lfoShape,      3.0f },
            { ParamIDs::lfoTarget,     0.0f },
            { ParamIDs::stereoWidth,   120.0f },
            { ParamIDs::outputLevel,   -2.0f },
            { ParamIDs::dryWet,        60.0f },
            { ParamIDs::bufferLength,  2.0f },
        }});

        // --- 6) Dark Drone ---
        presets.push_back ({ "Dark Drone", {
            { ParamIDs::grainSize,     400.0f },
            { ParamIDs::grainDensity,  5.0f },
            { ParamIDs::grainPosition, 50.0f },
            { ParamIDs::grainPitch,    -12.0f },
            { ParamIDs::grainPan,      0.0f },
            { ParamIDs::posScatter,    25.0f },
            { ParamIDs::pitchScatter,  3.0f },
            { ParamIDs::panScatter,    40.0f },
            { ParamIDs::grainAttack,   45.0f },
            { ParamIDs::grainDecay,    45.0f },
            { ParamIDs::envelopeShape, 1.0f },
            { ParamIDs::freeze,        0.0f },
            { ParamIDs::reverse,       0.0f },
            { ParamIDs::feedback,      0.6f },
            { ParamIDs::shimmer,       0.0f },
            { ParamIDs::lowCut,        30.0f },
            { ParamIDs::highCut,       5000.0f },
            { ParamIDs::lfoRate,       0.05f },
            { ParamIDs::lfoDepth,      25.0f },
            { ParamIDs::lfoShape,      1.0f },
            { ParamIDs::lfoTarget,     4.0f },
            { ParamIDs::stereoWidth,   80.0f },
            { ParamIDs::outputLevel,   0.0f },
            { ParamIDs::dryWet,        80.0f },
            { ParamIDs::bufferLength,  10.0f },
        }});

        // --- 7) Crystal Rain ---
        presets.push_back ({ "Crystal Rain", {
            { ParamIDs::grainSize,     50.0f },
            { ParamIDs::grainDensity,  25.0f },
            { ParamIDs::grainPosition, 50.0f },
            { ParamIDs::grainPitch,    7.0f },
            { ParamIDs::grainPan,      0.0f },
            { ParamIDs::posScatter,    70.0f },
            { ParamIDs::pitchScatter,  20.0f },
            { ParamIDs::panScatter,    90.0f },
            { ParamIDs::grainAttack,   10.0f },
            { ParamIDs::grainDecay,    30.0f },
            { ParamIDs::envelopeShape, 2.0f },
            { ParamIDs::freeze,        0.0f },
            { ParamIDs::reverse,       0.0f },
            { ParamIDs::feedback,      0.2f },
            { ParamIDs::shimmer,       40.0f },
            { ParamIDs::lowCut,        500.0f },
            { ParamIDs::highCut,       18000.0f },
            { ParamIDs::lfoRate,       2.0f },
            { ParamIDs::lfoDepth,      35.0f },
            { ParamIDs::lfoShape,      0.0f },
            { ParamIDs::lfoTarget,     3.0f },
            { ParamIDs::stereoWidth,   190.0f },
            { ParamIDs::outputLevel,   -2.0f },
            { ParamIDs::dryWet,        65.0f },
            { ParamIDs::bufferLength,  3.0f },
        }});

        // --- 8) Reverse Wash ---
        presets.push_back ({ "Reverse Wash", {
            { ParamIDs::grainSize,     350.0f },
            { ParamIDs::grainDensity,  8.0f },
            { ParamIDs::grainPosition, 50.0f },
            { ParamIDs::grainPitch,    0.0f },
            { ParamIDs::grainPan,      0.0f },
            { ParamIDs::posScatter,    50.0f },
            { ParamIDs::pitchScatter,  5.0f },
            { ParamIDs::panScatter,    55.0f },
            { ParamIDs::grainAttack,   10.0f },
            { ParamIDs::grainDecay,    50.0f },
            { ParamIDs::envelopeShape, 0.0f },
            { ParamIDs::freeze,        0.0f },
            { ParamIDs::reverse,       1.0f },
            { ParamIDs::feedback,      0.35f },
            { ParamIDs::shimmer,       20.0f },
            { ParamIDs::lowCut,        60.0f },
            { ParamIDs::highCut,       14000.0f },
            { ParamIDs::lfoRate,       0.3f },
            { ParamIDs::lfoDepth,      30.0f },
            { ParamIDs::lfoShape,      0.0f },
            { ParamIDs::lfoTarget,     1.0f },
            { ParamIDs::stereoWidth,   140.0f },
            { ParamIDs::outputLevel,   -1.0f },
            { ParamIDs::dryWet,        75.0f },
            { ParamIDs::bufferLength,  7.0f },
        }});
    }

    juce::AudioProcessorValueTreeState& valueTreeState;
    std::vector<Preset> presets;

    juce::Label      titleLabel;
    juce::ComboBox   presetCombo;
    juce::TextButton prevButton;
    juce::TextButton nextButton;
    juce::TextButton saveButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};
