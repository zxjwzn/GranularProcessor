/*
  ==============================================================================
    GrainScheduler.h
    Schedules grain creation based on density and scatter parameters.
  ==============================================================================
*/

#pragma once

#include "GrainPool.h"
#include "CircularBuffer.h"
#include <juce_core/juce_core.h>

class GrainScheduler
{
public:
    GrainScheduler() = default;

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        samplesUntilNextGrain = 0;
    }

    /** Call once per sample to potentially schedule a new grain.
        All parameter values should be pre-modulated (after LFO etc). */
    void process (GrainPool& pool, const CircularBuffer& circBuffer,
                  float grainSizeMs, float density,
                  float position, float posScatter,
                  float pitch, float pitchScatter,
                  float pan, float panScatter,
                  float attackFrac, float decayFrac,
                  EnvelopeShape envShape, bool reverse)
    {
        --samplesUntilNextGrain;

        if (samplesUntilNextGrain <= 0)
        {
            // Schedule next grain
            const float intervalSamples = static_cast<float> (sr) / juce::jmax (0.1f, density);
            samplesUntilNextGrain = static_cast<int> (intervalSamples);

            // Try to acquire a grain
            Grain* g = pool.acquire();
            if (g == nullptr)
                return; // Pool exhausted

            // Grain duration
            const float sizeSamples = (grainSizeMs / 1000.0f) * static_cast<float> (sr);
            g->lengthSamples = juce::jmax (1, static_cast<int> (sizeSamples));

            // Start position in circular buffer — RELATIVE to write head
            // position=0% reads from recent data, position=100% reads oldest data
            const float bufLen = static_cast<float> (circBuffer.getActiveLength());
            const int writePos = circBuffer.getWritePosition();
            const float lookbackAmount = (position / 100.0f) * bufLen;
            const float scatterRange = (posScatter / 100.0f) * bufLen * 0.5f;
            const float randomOffset = (random.nextFloat() * 2.0f - 1.0f) * scatterRange;
            const float rawPos = static_cast<float> (writePos) - lookbackAmount + randomOffset;
            g->startPos = std::fmod (rawPos + bufLen * 2.0f, bufLen);  // ensure positive

            // Pitch (semitones → playback rate)
            const float pitchRand = (random.nextFloat() * 2.0f - 1.0f) * (pitchScatter / 100.0f) * 12.0f;
            const float totalPitch = pitch + pitchRand;
            g->playbackRate = std::pow (2.0f, totalPitch / 12.0f);

            // Pan
            const float panRand = (random.nextFloat() * 2.0f - 1.0f) * (panScatter / 100.0f);
            g->pan = juce::jlimit (-1.0f, 1.0f, pan + panRand);

            // Envelope
            g->attackFrac = attackFrac / 100.0f;
            g->decayFrac  = decayFrac / 100.0f;
            g->envShape   = envShape;

            // Reverse
            g->reversed = reverse;

            // Reset playback
            g->currentSample = 0;
            g->gain = 1.0f;
        }
    }

    void reset()
    {
        samplesUntilNextGrain = 0;
    }

private:
    double sr = 44100.0;
    int samplesUntilNextGrain = 0;
    juce::Random random;
};
