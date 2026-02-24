/*
  ==============================================================================
    GrainEnvelope.h
    Window functions for grain amplitude envelopes.
  ==============================================================================
*/

#pragma once

#include <cmath>
#include <juce_core/juce_core.h>

enum class EnvelopeShape
{
    Hanning = 0,
    Gaussian,
    Triangle,
    Trapezoid
};

namespace GrainEnvelope
{
    /** Get envelope amplitude at normalised position [0, 1], with attack/decay fractions. */
    inline float getAmplitude (float normPos, float attackFrac, float decayFrac, EnvelopeShape shape)
    {
        // Clamp
        normPos    = juce::jlimit (0.0f, 1.0f, normPos);
        attackFrac = juce::jlimit (0.01f, 0.99f, attackFrac);
        decayFrac  = juce::jlimit (0.01f, 0.99f, decayFrac);

        // Ensure attack + decay don't exceed 1.0
        const float totalEnv = attackFrac + decayFrac;
        const float att = totalEnv > 1.0f ? attackFrac / totalEnv : attackFrac;
        const float dec = totalEnv > 1.0f ? decayFrac / totalEnv : decayFrac;

        float envGain = 1.0f;

        // Attack / sustain / decay regions
        if (normPos < att)
            envGain = normPos / att;
        else if (normPos > (1.0f - dec))
            envGain = (1.0f - normPos) / dec;

        // Apply window shape to the linear ramp
        switch (shape)
        {
            case EnvelopeShape::Hanning:
            {
                // Apply cosine shaping to the linear envelope
                return 0.5f * (1.0f - std::cos (juce::MathConstants<float>::pi * envGain));
            }
            case EnvelopeShape::Gaussian:
            {
                // Gaussian with sigma ~ 0.4
                const float x = (envGain - 1.0f);
                return std::exp (-0.5f * (x * x) / (0.16f));
            }
            case EnvelopeShape::Triangle:
            {
                return envGain;
            }
            case EnvelopeShape::Trapezoid:
            {
                // Flatten the middle, steeper attack/decay
                return juce::jlimit (0.0f, 1.0f, envGain * 1.5f);
            }
            default:
                return envGain;
        }
    }
}
