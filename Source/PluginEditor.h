/*
  ==============================================================================
    PluginEditor.h
    Granular Processor — Main editor with scalable UI.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/CustomLookAndFeel.h"
#include "UI/CustomKnob.h"
#include "UI/SectionPanel.h"
#include "UI/GlowToggleButton.h"
#include "UI/PresetBar.h"
#include "UI/ParticleVisualizer.h"
#include "Utils/ParamIDs.h"
#include "Utils/Constants.h"

class GranularProcessorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    GranularProcessorAudioProcessorEditor (GranularProcessorAudioProcessor&);
    ~GranularProcessorAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    GranularProcessorAudioProcessor& audioProcessor;

    CustomLookAndFeel customLnf;

    // Top bar
    PresetBar presetBar;

    // Visualizer
    ParticleVisualizer visualizer;

    // Section panels
    SectionPanel grainPanel   { "Grain" };
    SectionPanel scatterPanel { "Scatter" };
    SectionPanel envelopePanel { "Envelope" };
    SectionPanel effectsPanel { "Effects" };
    SectionPanel modulationPanel { "Modulation" };
    SectionPanel outputPanel  { "Output" };
    SectionPanel controlPanel { "Control" };

    // Grain knobs
    CustomKnob knobGrainSize    { "Size", "ms" };
    CustomKnob knobDensity      { "Density", "Hz" };
    CustomKnob knobPosition     { "Position", "%" };
    CustomKnob knobPitch        { "Pitch", "st" };
    CustomKnob knobPan          { "Pan" };

    // Scatter knobs
    CustomKnob knobPosScatter   { "Pos", "%" };
    CustomKnob knobPitchScatter { "Pitch", "%" };
    CustomKnob knobPanScatter   { "Pan", "%" };

    // Envelope knobs
    CustomKnob knobAttack       { "Attack", "%" };
    CustomKnob knobDecay        { "Decay", "%" };
    juce::ComboBox comboEnvShape;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> envShapeAttachment;

    // Effects knobs
    CustomKnob knobFeedback     { "Feedback" };
    CustomKnob knobShimmer      { "Shimmer", "%" };
    CustomKnob knobLowCut       { "Low Cut", "Hz" };
    CustomKnob knobHighCut      { "High Cut", "Hz" };

    // Modulation knobs
    CustomKnob knobLFORate      { "Rate", "Hz" };
    CustomKnob knobLFODepth     { "Depth", "%" };
    juce::ComboBox comboLFOShape;
    juce::ComboBox comboLFOTarget;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoShapeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoTargetAttachment;

    // Output knobs
    CustomKnob knobWidth        { "Width", "%" };
    CustomKnob knobLevel        { "Level", "dB" };
    CustomKnob knobMix          { "Mix", "%" };
    CustomKnob knobBuffer       { "Buffer", "s" };

    // Control buttons
    GlowToggleButton btnFreeze  { "FREEZE", Theme::accentGreen };
    GlowToggleButton btnReverse { "REVERSE", Theme::primaryPurple };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularProcessorAudioProcessorEditor)
};
