/*
  ==============================================================================
    ParameterLayout.cpp
    Creates the AudioProcessorValueTreeState parameter layout.
  ==============================================================================
*/

#include "ParameterLayout.h"
#include "ParamIDs.h"
#include "Constants.h"

using namespace GranularConstants;

juce::AudioProcessorValueTreeState::ParameterLayout ParameterLayout::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ===== Core Grain =====
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::grainSize, 1 }, "Grain Size",
        juce::NormalisableRange<float> (kMinGrainSizeMs, kMaxGrainSizeMs, 0.1f, 0.5f),
        kDefaultGrainSize,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::grainDensity, 1 }, "Density",
        juce::NormalisableRange<float> (kMinDensity, kMaxDensity, 0.1f, 0.5f),
        kDefaultDensity,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::grainPosition, 1 }, "Position",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::grainPitch, 1 }, "Pitch",
        juce::NormalisableRange<float> (kMinPitch, kMaxPitch, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("st")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::grainPan, 1 }, "Pan",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f),
        0.0f));

    // ===== Scatter =====
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::posScatter, 1 }, "Pos Scatter",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        20.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::pitchScatter, 1 }, "Pitch Scatter",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::panScatter, 1 }, "Pan Scatter",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        30.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    // ===== Envelope =====
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::grainAttack, 1 }, "Attack",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        25.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::grainDecay, 1 }, "Decay",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        25.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::envelopeShape, 1 }, "Env Shape",
        juce::StringArray { "Hanning", "Gaussian", "Triangle", "Trapezoid" },
        0));

    // ===== Effects =====
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::freeze, 1 }, "Freeze", false));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::reverse, 1 }, "Reverse", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::feedback, 1 }, "Feedback",
        juce::NormalisableRange<float> (0.0f, kMaxFeedback, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::shimmer, 1 }, "Shimmer",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::lowCut, 1 }, "Low Cut",
        juce::NormalisableRange<float> (kMinLowCut, kMaxLowCut, 1.0f, 0.3f),
        20.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::highCut, 1 }, "High Cut",
        juce::NormalisableRange<float> (kMinHighCut, kMaxHighCut, 1.0f, 0.3f),
        20000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    // ===== LFO =====
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::lfoRate, 1 }, "LFO Rate",
        juce::NormalisableRange<float> (kMinLFORate, kMaxLFORate, 0.01f, 0.3f),
        1.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::lfoDepth, 1 }, "LFO Depth",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::lfoShape, 1 }, "LFO Shape",
        juce::StringArray { "Sine", "Triangle", "Square", "S&&H" },
        0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::lfoTarget, 1 }, "LFO Target",
        juce::StringArray { "Size", "Position", "Pitch", "Pan", "Filter" },
        1));

    // ===== Output =====
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::stereoWidth, 1 }, "Stereo Width",
        juce::NormalisableRange<float> (0.0f, 200.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::outputLevel, 1 }, "Output Level",
        juce::NormalisableRange<float> (-60.0f, 6.0f, 0.1f, 2.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::dryWet, 1 }, "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::bufferLength, 1 }, "Buffer Length",
        juce::NormalisableRange<float> (kMinBufferSeconds, kMaxBufferSeconds, 0.1f),
        kDefaultBufferSec,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    return { params.begin(), params.end() };
}
