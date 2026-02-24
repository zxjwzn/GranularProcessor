/*
  ==============================================================================
    GranularEngine.h
    Main granular processing engine — integrates buffer, scheduler, pool,
    LFO, and post-processing.
  ==============================================================================
*/

#pragma once

#include "CircularBuffer.h"
#include "GrainPool.h"
#include "GrainScheduler.h"
#include "LFOModulator.h"
#include "PostProcessor.h"
#include "../Utils/Constants.h"
#include "../Utils/ParamIDs.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <atomic>
#include <array>
#include <vector>
#include <cmath>

/** Snapshot of active grains for the visualizer (lock-free transfer). */
struct GrainVisualData
{
    struct GrainInfo
    {
        bool  active        = false;
        float normPosition  = 0.0f;   // 0-1 in buffer
        float envelope      = 0.0f;   // current amplitude
        float pitch         = 0.0f;   // semitones
        float pan           = 0.0f;   // -1..1
        float size          = 0.0f;   // normalised grain size
    };
    std::array<GrainInfo, GranularConstants::kMaxGrains> grains;
    int activeCount = 0;
    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
};

class GranularEngine
{
public:
    GranularEngine() = default;

    void prepare (double sampleRate, int samplesPerBlock, int numChannels)
    {
        sr = sampleRate;
        blockSize = samplesPerBlock;
        channels = numChannels;

        circularBuffer.prepare (sampleRate, numChannels, GranularConstants::kMaxBufferSeconds);
        scheduler.prepare (sampleRate);
        lfo.prepare (sampleRate);

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
        spec.numChannels = static_cast<juce::uint32> (numChannels);
        postProcessor.prepare (spec);

        grainOutput.setSize (numChannels, samplesPerBlock);
        shimmerFeedback.setSize (numChannels, samplesPerBlock);

        // Pre-allocate write position tracking
        writePositions.resize (static_cast<size_t> (samplesPerBlock), 0);

        pool.resetAll();
        scheduler.reset();
        lfo.reset();

        smoothedDryWet.reset (sampleRate, 0.02);
        smoothedOutputLevel.reset (sampleRate, 0.02);
    }

    void process (juce::AudioBuffer<float>& buffer,
                  juce::AudioProcessorValueTreeState& apvts)
    {
        const int numSamples  = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();

        // Read parameter values
        const float grainSizeMs = apvts.getRawParameterValue (ParamIDs::grainSize)->load();
        const float density     = apvts.getRawParameterValue (ParamIDs::grainDensity)->load();
        const float position    = apvts.getRawParameterValue (ParamIDs::grainPosition)->load();
        const float pitch       = apvts.getRawParameterValue (ParamIDs::grainPitch)->load();
        const float pan         = apvts.getRawParameterValue (ParamIDs::grainPan)->load();
        const float posScatter  = apvts.getRawParameterValue (ParamIDs::posScatter)->load();
        const float pitchScatter = apvts.getRawParameterValue (ParamIDs::pitchScatter)->load();
        const float panScatter  = apvts.getRawParameterValue (ParamIDs::panScatter)->load();
        const float attack      = apvts.getRawParameterValue (ParamIDs::grainAttack)->load();
        const float decay       = apvts.getRawParameterValue (ParamIDs::grainDecay)->load();
        const int   envShapeIdx = static_cast<int> (apvts.getRawParameterValue (ParamIDs::envelopeShape)->load());
        const bool  freezeOn    = apvts.getRawParameterValue (ParamIDs::freeze)->load() > 0.5f;
        const bool  reverseOn   = apvts.getRawParameterValue (ParamIDs::reverse)->load() > 0.5f;
        const float feedbackAmt = apvts.getRawParameterValue (ParamIDs::feedback)->load();
        const float shimmerAmt  = apvts.getRawParameterValue (ParamIDs::shimmer)->load();
        const float lowCut      = apvts.getRawParameterValue (ParamIDs::lowCut)->load();
        const float highCut     = apvts.getRawParameterValue (ParamIDs::highCut)->load();
        const float lfoRate     = apvts.getRawParameterValue (ParamIDs::lfoRate)->load();
        const float lfoDepth    = apvts.getRawParameterValue (ParamIDs::lfoDepth)->load();
        const int   lfoShapeIdx = static_cast<int> (apvts.getRawParameterValue (ParamIDs::lfoShape)->load());
        const int   lfoTargetIdx = static_cast<int> (apvts.getRawParameterValue (ParamIDs::lfoTarget)->load());
        const float stereoWidth = apvts.getRawParameterValue (ParamIDs::stereoWidth)->load();
        const float outLevel    = apvts.getRawParameterValue (ParamIDs::outputLevel)->load();
        const float dryWet      = apvts.getRawParameterValue (ParamIDs::dryWet)->load();
        const float bufLenSec   = apvts.getRawParameterValue (ParamIDs::bufferLength)->load();

        const auto envShape  = static_cast<EnvelopeShape> (envShapeIdx);
        const auto lfoShape  = static_cast<LFOShape> (lfoShapeIdx);
        const auto lfoTarget = static_cast<LFOTarget> (lfoTargetIdx);

        // Update buffer length and freeze state
        circularBuffer.setBufferLength (bufLenSec);
        circularBuffer.setFrozen (freezeOn);

        // Set smoothed values
        smoothedDryWet.setTargetValue (dryWet / 100.0f);
        smoothedOutputLevel.setTargetValue (juce::Decibels::decibelsToGain (outLevel));

        // Measure input level for visualizer
        float inLevelSum = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            inLevelSum += buffer.getRMSLevel (ch, 0, numSamples);

        // Prepare grain output buffer
        grainOutput.setSize (numChannels, numSamples, false, false, true);
        grainOutput.clear();

        // Ensure write position tracking is large enough
        if (static_cast<int> (writePositions.size()) < numSamples)
            writePositions.resize (static_cast<size_t> (numSamples), 0);

        // Process sample by sample
        for (int s = 0; s < numSamples; ++s)
        {
            // Record write position BEFORE writing (for feedback later)
            writePositions[static_cast<size_t> (s)] = circularBuffer.getWritePosition();

            // Write input to circular buffer
            for (int ch = 0; ch < numChannels; ++ch)
                circularBuffer.writeSample (ch, buffer.getSample (ch, s));
            circularBuffer.advanceWritePosition();

            // LFO modulation
            const float lfoValue = lfo.process (lfoRate, lfoShape) * (lfoDepth / 100.0f);

            // Apply LFO to target parameter
            float modGrainSize = grainSizeMs;
            float modPosition  = position;
            float modPitch     = pitch;
            float modPan       = pan;

            switch (lfoTarget)
            {
                case LFOTarget::Size:
                    modGrainSize *= (1.0f + lfoValue * 0.5f);
                    break;
                case LFOTarget::Position:
                    modPosition += lfoValue * 30.0f;
                    break;
                case LFOTarget::Pitch:
                    modPitch += lfoValue * 12.0f;
                    break;
                case LFOTarget::Pan:
                    modPan = juce::jlimit (-1.0f, 1.0f, modPan + lfoValue);
                    break;
                case LFOTarget::Filter:
                    // Filter modulation is handled later in post-processing
                    break;
            }

            modGrainSize = juce::jlimit (GranularConstants::kMinGrainSizeMs,
                                          GranularConstants::kMaxGrainSizeMs, modGrainSize);
            modPosition = juce::jlimit (0.0f, 100.0f, modPosition);

            // Schedule new grains
            scheduler.process (pool, circularBuffer,
                              modGrainSize, density,
                              modPosition, posScatter,
                              modPitch, pitchScatter,
                              modPan, panScatter,
                              attack, decay,
                              envShape, reverseOn);

            // Process all active grains and sum their output
            float mixL = 0.0f, mixR = 0.0f;
            int activeGrainCount = 0;

            pool.processAll ([&] (Grain& grain)
            {
                const float readPos = grain.getReadPosition();
                const float envAmp  = grain.getEnvelopeAmplitude();

                // Read from circular buffer
                float sampleL = circularBuffer.readSample (0, readPos);
                float sampleR = numChannels > 1 ? circularBuffer.readSample (1, readPos) : sampleL;

                // Apply envelope and gain
                sampleL *= envAmp * grain.gain;
                sampleR *= envAmp * grain.gain;

                // Apply panning (constant power)
                const float panAngle = (grain.pan + 1.0f) * 0.5f; // 0-1
                const float panL = std::cos (panAngle * juce::MathConstants<float>::halfPi);
                const float panR = std::sin (panAngle * juce::MathConstants<float>::halfPi);

                mixL += sampleL * panL;
                mixR += sampleR * panR;

                ++activeGrainCount;
                grain.advance();
            });

            // Normalize by active grain count to prevent volume explosion
            // Use sqrt scaling for more natural summing behavior
            if (activeGrainCount > 1)
            {
                const float normFactor = 1.0f / std::sqrt (static_cast<float> (activeGrainCount));
                mixL *= normFactor;
                mixR *= normFactor;
            }

            // Write grain mix to output
            grainOutput.setSample (0, s, mixL);
            if (numChannels > 1)
                grainOutput.setSample (1, s, mixR);
        }

        // Post-processing (filters, DC blocker, width, shimmer, soft clip)
        postProcessor.process (grainOutput, lowCut, highCut, stereoWidth, shimmerAmt, shimmerFeedback);

        // Write feedback back into circular buffer at the CORRECT per-sample positions
        if (feedbackAmt > 0.001f)
        {
            for (int s = 0; s < numSamples; ++s)
            {
                const int fbPos = writePositions[static_cast<size_t> (s)];
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    const float fbSample = grainOutput.getSample (ch, s) * feedbackAmt;
                    circularBuffer.writeFeedbackAt (ch, fbPos, fbSample);
                }
            }
        }

        // Measure output level for visualizer
        float outLevelSum = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            outLevelSum += grainOutput.getRMSLevel (ch, 0, numSamples);

        // Dry/Wet mix and output level
        for (int s = 0; s < numSamples; ++s)
        {
            const float wet = smoothedDryWet.getNextValue();
            const float dry = 1.0f - wet;
            const float level = smoothedOutputLevel.getNextValue();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float drySample = buffer.getSample (ch, s);
                const float wetSample = grainOutput.getSample (ch, s);
                buffer.setSample (ch, s, (drySample * dry + wetSample * wet) * level);
            }
        }

        // Update visual data (lock-free)
        updateVisualData (inLevelSum / static_cast<float> (numChannels),
                          outLevelSum / static_cast<float> (numChannels));
    }

    /** Get latest visual data for the UI (called from message thread). */
    GrainVisualData getVisualData() const
    {
        return visualData.load();
    }

    void reset()
    {
        pool.resetAll();
        scheduler.reset();
        lfo.reset();
        postProcessor.reset();
    }

private:
    void updateVisualData (float inLevel, float outLevel)
    {
        GrainVisualData data;
        data.inputLevel = inLevel;
        data.outputLevel = outLevel;
        data.activeCount = 0;

        const auto& grains = pool.getGrains();
        const float bufLen = static_cast<float> (circularBuffer.getActiveLength());
        const float maxSize = GranularConstants::kMaxGrainSizeMs / 1000.0f * static_cast<float> (sr);

        for (int i = 0; i < GranularConstants::kMaxGrains; ++i)
        {
            const auto& g = grains[static_cast<size_t> (i)];
            data.grains[static_cast<size_t> (i)].active = g.active;

            if (g.active)
            {
                data.grains[static_cast<size_t> (i)].normPosition = 
                    bufLen > 0.0f ? std::fmod (g.getReadPosition(), bufLen) / bufLen : 0.0f;
                data.grains[static_cast<size_t> (i)].envelope = g.getEnvelopeAmplitude();
                data.grains[static_cast<size_t> (i)].pitch = 
                    12.0f * std::log2 (std::max (0.001f, g.playbackRate));
                data.grains[static_cast<size_t> (i)].pan = g.pan;
                data.grains[static_cast<size_t> (i)].size = 
                    maxSize > 0.0f ? static_cast<float> (g.lengthSamples) / maxSize : 0.0f;
                ++data.activeCount;
            }
        }

        visualData.store (data);
    }

    double sr = 44100.0;
    int blockSize = 512;
    int channels = 2;

    CircularBuffer    circularBuffer;
    GrainPool         pool;
    GrainScheduler    scheduler;
    LFOModulator      lfo;
    PostProcessor     postProcessor;

    juce::AudioBuffer<float> grainOutput;
    juce::AudioBuffer<float> shimmerFeedback;

    // Track write positions per sample for correct feedback
    std::vector<int> writePositions;

    juce::SmoothedValue<float> smoothedDryWet  { 0.5f };
    juce::SmoothedValue<float> smoothedOutputLevel { 1.0f };

    std::atomic<GrainVisualData> visualData;
};
