/*
  ==============================================================================
    LFOModulator.h
    Low-frequency oscillator for parameter modulation.
  ==============================================================================
*/

#pragma once

#include <cmath>
#include <juce_core/juce_core.h>

enum class LFOShape
{
    Sine = 0,
    Triangle,
    Square,
    SampleAndHold
};

enum class LFOTarget
{
    Size = 0,
    Position,
    Pitch,
    Pan,
    Filter
};

class LFOModulator
{
public:
    LFOModulator() = default;

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0f;
        currentSHValue = 0.0f;
    }

    /** Advance by one sample and return the modulation value in [-1, 1].
        rate in Hz, depth in [0, 1]. */
    float process (float rate, LFOShape shape)
    {
        // Advance phase
        const float phaseInc = rate / static_cast<float> (sr);
        phase += phaseInc;
        if (phase >= 1.0f) 
        {
            phase -= 1.0f;
            // Trigger new S&H value at phase reset
            shTriggered = true;
        }

        float value = 0.0f;

        switch (shape)
        {
            case LFOShape::Sine:
                value = std::sin (phase * juce::MathConstants<float>::twoPi);
                break;

            case LFOShape::Triangle:
                value = 2.0f * std::abs (2.0f * phase - 1.0f) - 1.0f;
                break;

            case LFOShape::Square:
                value = phase < 0.5f ? 1.0f : -1.0f;
                break;

            case LFOShape::SampleAndHold:
                if (shTriggered)
                {
                    currentSHValue = random.nextFloat() * 2.0f - 1.0f;
                    shTriggered = false;
                }
                value = currentSHValue;
                break;
        }

        return value;
    }

    void reset()
    {
        phase = 0.0f;
        currentSHValue = 0.0f;
        shTriggered = false;
    }

private:
    double sr = 44100.0;
    float  phase = 0.0f;
    float  currentSHValue = 0.0f;
    bool   shTriggered = false;
    juce::Random random;
};
