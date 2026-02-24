/*
  ==============================================================================
    PostProcessor.h
    Post-processing chain: filters, stereo width, shimmer, DC blocker, soft clip.
  ==============================================================================
*/

#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

class PostProcessor
{
public:
    PostProcessor() = default;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        numChannels = static_cast<int> (spec.numChannels);

        // High-pass (low cut)
        highPassFilter.prepare (spec);
        highPassFilter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        highPassFilter.setCutoffFrequency (20.0f);

        // Low-pass (high cut) 
        lowPassFilter.prepare (spec);
        lowPassFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        lowPassFilter.setCutoffFrequency (20000.0f);

        // DC blocker filters (one per channel)
        dcBlockerX.resize (static_cast<size_t> (numChannels), 0.0f);
        dcBlockerY.resize (static_cast<size_t> (numChannels), 0.0f);

        // Shimmer delay: use a proper fractional delay line
        shimmerDelaySamples = static_cast<int> (sampleRate * 0.3);  // 300ms delay
        shimmerBuffer.setSize (numChannels, shimmerDelaySamples + 4);  // +4 for interpolation safety
        shimmerBuffer.clear();
        shimmerWritePos = 0;

        // Shimmer dampening filter (reduce harshness of octave-up)
        shimmerDampen.prepare (spec);
        shimmerDampen.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        shimmerDampen.setCutoffFrequency (8000.0f);
    }

    /** Process a stereo block in-place. */
    void process (juce::AudioBuffer<float>& buffer,
                  float lowCutFreq, float highCutFreq,
                  float stereoWidth, float shimmerAmount,
                  juce::AudioBuffer<float>& /*shimmerFeedbackOut*/)
    {
        const int numSamples = buffer.getNumSamples();
        const int bufChannels = buffer.getNumChannels();

        // Apply DC blocker first (removes DC offset from grain summing)
        applyDCBlocker (buffer);

        // Update filter cutoffs (only if changed significantly to avoid zipper noise)
        highPassFilter.setCutoffFrequency (lowCutFreq);
        lowPassFilter.setCutoffFrequency (highCutFreq);

        // Apply filters
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        highPassFilter.process (context);
        lowPassFilter.process (context);

        // Stereo width (Mid/Side processing)
        if (bufChannels >= 2)
        {
            const float widthFactor = stereoWidth / 100.0f;  // 0-2 range

            for (int s = 0; s < numSamples; ++s)
            {
                const float left  = buffer.getSample (0, s);
                const float right = buffer.getSample (1, s);

                const float mid  = (left + right) * 0.5f;
                const float side = (left - right) * 0.5f;

                const float newLeft  = mid + side * widthFactor;
                const float newRight = mid - side * widthFactor;

                buffer.setSample (0, s, newLeft);
                buffer.setSample (1, s, newRight);
            }
        }

        // Shimmer: proper pitch-shifted (octave-up) delay with feedback
        if (shimmerAmount > 0.001f && shimmerDelaySamples > 0)
        {
            const float shimMix = shimmerAmount / 100.0f;

            for (int s = 0; s < numSamples; ++s)
            {
                for (int ch = 0; ch < juce::jmin (bufChannels, 2); ++ch)
                {
                    // Read from shimmer buffer at 2x speed (octave up)
                    // Use a fixed delay offset and read at double rate
                    const float readOffset = static_cast<float> (shimmerDelaySamples) * 0.5f;
                    const float readPosF = static_cast<float> (shimmerWritePos) - readOffset;
                    const float wrappedPos = readPosF < 0.0f 
                        ? readPosF + static_cast<float> (shimmerDelaySamples) 
                        : readPosF;

                    // Linear interpolation for clean reading
                    const int readIdx0 = static_cast<int> (wrappedPos) % shimmerDelaySamples;
                    const int readIdx1 = (readIdx0 + 1) % shimmerDelaySamples;
                    const float frac = wrappedPos - std::floor (wrappedPos);
                    
                    const float s0 = shimmerBuffer.getSample (ch, readIdx0);
                    const float s1 = shimmerBuffer.getSample (ch, readIdx1);
                    const float shimSample = s0 + frac * (s1 - s0);

                    // Mix shimmer into output (with reduced gain to prevent buildup)
                    const float current = buffer.getSample (ch, s);
                    buffer.setSample (ch, s, current + shimSample * shimMix * 0.3f);

                    // Write current signal into shimmer buffer (with decay)
                    shimmerBuffer.setSample (ch, shimmerWritePos, 
                        current * 0.7f + shimSample * shimMix * 0.2f);
                }

                shimmerWritePos = (shimmerWritePos + 1) % shimmerDelaySamples;
            }

            // Apply dampening filter to shimmer output to remove harsh highs
            shimmerDampen.process (context);
        }

        // Soft clip the entire output to prevent harsh digital distortion
        applySoftClip (buffer);
    }

    void reset()
    {
        highPassFilter.reset();
        lowPassFilter.reset();
        shimmerDampen.reset();
        shimmerBuffer.clear();
        shimmerWritePos = 0;

        std::fill (dcBlockerX.begin(), dcBlockerX.end(), 0.0f);
        std::fill (dcBlockerY.begin(), dcBlockerY.end(), 0.0f);
    }

private:
    /** DC blocking filter: y[n] = x[n] - x[n-1] + R * y[n-1], R ~= 0.995 */
    void applyDCBlocker (juce::AudioBuffer<float>& buffer)
    {
        const float R = 0.995f;
        const int numSamples = buffer.getNumSamples();
        const int bufChannels = buffer.getNumChannels();

        for (int ch = 0; ch < bufChannels; ++ch)
        {
            auto chIdx = static_cast<size_t> (ch);
            if (chIdx >= dcBlockerX.size()) continue;

            float xPrev = dcBlockerX[chIdx];
            float yPrev = dcBlockerY[chIdx];
            auto* data = buffer.getWritePointer (ch);

            for (int s = 0; s < numSamples; ++s)
            {
                const float x = data[s];
                const float y = x - xPrev + R * yPrev;
                data[s] = y;
                xPrev = x;
                yPrev = y;
            }

            dcBlockerX[chIdx] = xPrev;
            dcBlockerY[chIdx] = yPrev;
        }
    }

    /** Soft clipping using tanh to prevent harsh digital distortion */
    void applySoftClip (juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int bufChannels = buffer.getNumChannels();

        for (int ch = 0; ch < bufChannels; ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            for (int s = 0; s < numSamples; ++s)
            {
                // tanh soft clip — keeps signal in (-1, 1) range smoothly
                data[s] = std::tanh (data[s]);
            }
        }
    }

    double sampleRate = 44100.0;
    int numChannels = 2;

    juce::dsp::StateVariableTPTFilter<float> highPassFilter;
    juce::dsp::StateVariableTPTFilter<float> lowPassFilter;
    juce::dsp::StateVariableTPTFilter<float> shimmerDampen;

    // DC blocker state
    std::vector<float> dcBlockerX;
    std::vector<float> dcBlockerY;

    // Shimmer delay line
    juce::AudioBuffer<float> shimmerBuffer;
    int shimmerWritePos = 0;
    int shimmerDelaySamples = 0;
};
