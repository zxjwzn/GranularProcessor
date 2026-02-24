/*
  ==============================================================================
    Grain.h
    Single grain structure — lightweight POD-style, no virtual functions.
  ==============================================================================
*/

#pragma once

#include "GrainEnvelope.h"

struct Grain
{
    bool   active       = false;

    // Read position in the circular buffer (fractional samples)
    float  startPos     = 0.0f;
    float  currentPos   = 0.0f;

    // Duration in samples
    int    lengthSamples = 0;
    int    currentSample = 0;

    // Playback rate (1.0 = original pitch, 2.0 = octave up, 0.5 = octave down)
    float  playbackRate = 1.0f;

    // Panning (-1 = left, 0 = center, 1 = right)
    float  pan          = 0.0f;

    // Envelope
    float  attackFrac   = 0.25f;
    float  decayFrac    = 0.25f;
    EnvelopeShape envShape = EnvelopeShape::Hanning;

    // Reverse playback
    bool   reversed     = false;

    // Gain (applied after envelope)
    float  gain         = 1.0f;

    /** Get the normalised position within the grain [0, 1] */
    float getNormalisedPosition() const
    {
        return lengthSamples > 0 ? static_cast<float> (currentSample) / static_cast<float> (lengthSamples) : 0.0f;
    }

    /** Get the current envelope amplitude */
    float getEnvelopeAmplitude() const
    {
        return GrainEnvelope::getAmplitude (getNormalisedPosition(), attackFrac, decayFrac, envShape);
    }

    /** Get the current read position in the circular buffer */
    float getReadPosition() const
    {
        if (reversed)
            return startPos - static_cast<float> (currentSample) * playbackRate;
        else
            return startPos + static_cast<float> (currentSample) * playbackRate;
    }

    /** Advance the grain by one sample, returns false when grain is done */
    bool advance()
    {
        if (! active) return false;
        ++currentSample;
        if (currentSample >= lengthSamples)
        {
            active = false;
            return false;
        }
        return true;
    }

    /** Reset the grain to inactive */
    void reset()
    {
        active = false;
        currentSample = 0;
        lengthSamples = 0;
    }
};
