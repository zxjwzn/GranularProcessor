/*
  ==============================================================================
    CircularBuffer.h
    Ring buffer with freeze support and fractional-sample reading.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../Utils/Constants.h"
#include <vector>

class CircularBuffer
{
public:
    CircularBuffer() = default;

    void prepare (double sampleRate, int numChannels, float maxLengthSeconds)
    {
        sr = sampleRate;
        channels = numChannels;
        maxSamples = static_cast<int> (sr * maxLengthSeconds);
        buffer.setSize (channels, maxSamples);
        buffer.clear();
        writePos = 0;
        activeSamples = maxSamples;
    }

    void setBufferLength (float lengthSeconds)
    {
        activeSamples = juce::jlimit (1, maxSamples,
                                      static_cast<int> (sr * lengthSeconds));
    }

    void writeSample (int channel, float sample)
    {
        if (frozen) return;
        buffer.setSample (channel, writePos % activeSamples, sample);
    }

    void advanceWritePosition()
    {
        if (frozen) return;
        writePos = (writePos + 1) % activeSamples;
    }

    /** Read with Hermite interpolation at a fractional sample position */
    float readSample (int channel, float fractionalPos) const
    {
        const int len = activeSamples;
        const float normalised = std::fmod (fractionalPos, static_cast<float> (len));
        const float pos = normalised < 0.0f ? normalised + static_cast<float> (len) : normalised;

        const int i0 = static_cast<int> (pos);
        const float frac = pos - static_cast<float> (i0);

        // 4-point Hermite interpolation
        const int im1 = (i0 - 1 + len) % len;
        const int i1  = (i0 + 1) % len;
        const int i2  = (i0 + 2) % len;

        const float ym1 = buffer.getSample (channel, im1);
        const float y0  = buffer.getSample (channel, i0);
        const float y1  = buffer.getSample (channel, i1);
        const float y2  = buffer.getSample (channel, i2);

        // Hermite interpolation formula
        const float c0 = y0;
        const float c1 = 0.5f * (y1 - ym1);
        const float c2 = ym1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
        const float c3 = 0.5f * (y2 - ym1) + 1.5f * (y0 - y1);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

    /** Write feedback signal into the buffer at a specific position (adds to existing content). */
    void writeFeedbackAt (int channel, int position, float sample)
    {
        if (frozen) return;
        const int pos = ((position % activeSamples) + activeSamples) % activeSamples;
        const float existing = buffer.getSample (channel, pos);
        // Soft-limit feedback to prevent runaway
        const float combined = existing + sample;
        buffer.setSample (channel, pos, combined);
    }

    int getWritePosition() const { return writePos; }
    int getActiveLength()  const { return activeSamples; }
    double getSampleRate() const { return sr; }
    bool isFrozen()        const { return frozen; }
    void setFrozen (bool shouldFreeze) { frozen = shouldFreeze; }

private:
    juce::AudioBuffer<float> buffer;
    double sr = 44100.0;
    int channels = 2;
    int maxSamples = 0;
    int activeSamples = 0;
    int writePos = 0;
    bool frozen = false;
};
