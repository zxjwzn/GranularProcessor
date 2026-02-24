/*
  ==============================================================================
    ParamIDs.h
    Centralized parameter ID strings for the Granular Processor plugin.
  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>

namespace ParamIDs
{
    // Core Grain
    inline const juce::String grainSize      { "grainSize" };
    inline const juce::String grainDensity   { "grainDensity" };
    inline const juce::String grainPosition  { "grainPosition" };
    inline const juce::String grainPitch     { "grainPitch" };
    inline const juce::String grainPan       { "grainPan" };

    // Scatter
    inline const juce::String posScatter     { "posScatter" };
    inline const juce::String pitchScatter   { "pitchScatter" };
    inline const juce::String panScatter     { "panScatter" };

    // Envelope
    inline const juce::String grainAttack    { "grainAttack" };
    inline const juce::String grainDecay     { "grainDecay" };
    inline const juce::String envelopeShape  { "envelopeShape" };

    // Effects
    inline const juce::String freeze         { "freeze" };
    inline const juce::String reverse        { "reverse" };
    inline const juce::String feedback       { "feedback" };
    inline const juce::String shimmer        { "shimmer" };
    inline const juce::String lowCut         { "lowCut" };
    inline const juce::String highCut        { "highCut" };

    // LFO
    inline const juce::String lfoRate        { "lfoRate" };
    inline const juce::String lfoDepth       { "lfoDepth" };
    inline const juce::String lfoShape       { "lfoShape" };
    inline const juce::String lfoTarget      { "lfoTarget" };

    // Output
    inline const juce::String stereoWidth    { "stereoWidth" };
    inline const juce::String outputLevel    { "outputLevel" };
    inline const juce::String dryWet         { "dryWet" };
    inline const juce::String bufferLength   { "bufferLength" };
}
