/*
  ==============================================================================
    Constants.h
    Global constants for the Granular Processor plugin.
  ==============================================================================
*/

#pragma once

namespace GranularConstants
{
    // Grain pool
    constexpr int    kMaxGrains         = 64;

    // Circular buffer
    constexpr float  kMinBufferSeconds  = 1.0f;
    constexpr float  kMaxBufferSeconds  = 10.0f;
    constexpr float  kDefaultBufferSec  = 4.0f;

    // Grain size (ms)
    constexpr float  kMinGrainSizeMs    = 10.0f;
    constexpr float  kMaxGrainSizeMs    = 500.0f;
    constexpr float  kDefaultGrainSize  = 100.0f;

    // Grain density (grains per second)
    constexpr float  kMinDensity        = 1.0f;
    constexpr float  kMaxDensity        = 50.0f;
    constexpr float  kDefaultDensity    = 8.0f;

    // Pitch (semitones)
    constexpr float  kMinPitch          = -24.0f;
    constexpr float  kMaxPitch          = 24.0f;

    // LFO
    constexpr float  kMinLFORate        = 0.01f;
    constexpr float  kMaxLFORate        = 20.0f;

    // Filter
    constexpr float  kMinLowCut         = 20.0f;
    constexpr float  kMaxLowCut         = 2000.0f;
    constexpr float  kMinHighCut        = 1000.0f;
    constexpr float  kMaxHighCut        = 20000.0f;

    // Feedback max (prevent runaway)
    constexpr float  kMaxFeedback       = 0.95f;

    // UI
    constexpr int    kBaseWidth         = 900;
    constexpr int    kBaseHeight        = 600;
    constexpr float  kMinScaleFactor    = 0.7f;
    constexpr float  kMaxScaleFactor    = 1.5f;

    // Visualizer
    constexpr int    kVisualizerFPS     = 60;
    constexpr int    kMaxVisualParticles = 128;

    // Smoothing ramp length (samples)
    constexpr int    kSmoothingRampLen  = 64;
}
