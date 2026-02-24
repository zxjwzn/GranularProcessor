/*
  ==============================================================================
    ParticleVisualizer.h
    Ambient particle visualization for active grains.
    Each grain maps to a persistent visual particle with smooth fade-in/out,
    gentle drift, multi-layered glow, and rich colour palette.
  ==============================================================================
*/

#pragma once

#include <juce_opengl/juce_opengl.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/GranularEngine.h"
#include "CustomLookAndFeel.h"
#include <array>
#include <cmath>

class ParticleVisualizer : public juce::Component,
                           public juce::OpenGLRenderer,
                           private juce::Timer
{
public:
    ParticleVisualizer()
    {
        openGLContext.setRenderer (this);
        openGLContext.attachTo (*this);
        openGLContext.setContinuousRepainting (false);

        startTimerHz (120);

        // Seed each visual particle with a stable random offset
        juce::Random rng (42);
        for (auto& vp : particles)
        {
            vp.driftPhaseX = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            vp.driftPhaseY = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            vp.driftSpeedX = 0.2f + rng.nextFloat() * 0.6f;   // slow drift
            vp.driftSpeedY = 0.15f + rng.nextFloat() * 0.5f;
            vp.hueOffset   = rng.nextFloat() * 0.15f - 0.075f; // slight per-particle colour variation
            vp.baseX       = 0.5f;
            vp.baseY       = 0.5f;
            vp.displayAlpha = 0.0f;
            vp.displaySize  = 0.0f;
            vp.wasActive    = false;
        }

        globalTime = 0.0f;
    }

    ~ParticleVisualizer() override
    {
        stopTimer();
        openGLContext.detach();
    }

    void updateGrainData (const GrainVisualData& data)
    {
        latestData.store (data);
    }

    // OpenGLRenderer callbacks
    void newOpenGLContextCreated() override {}
    void openGLContextClosing() override {}

    void renderOpenGL() override
    {
        juce::OpenGLHelpers::clear (Theme::background);

        const float w = static_cast<float> (getWidth());
        const float h = static_cast<float> (getHeight());

        if (w <= 0.0f || h <= 0.0f) return;

        std::unique_ptr<juce::LowLevelGraphicsContext> glContext (
            createOpenGLGraphicsContext (openGLContext, static_cast<int> (w), static_cast<int> (h)));

        if (glContext == nullptr) return;

        juce::Graphics g (*glContext);
        drawContent (g, w, h);
    }

    void paint (juce::Graphics& g) override
    {
        if (! openGLContext.isAttached())
        {
            g.fillAll (Theme::background);
            drawContent (g, static_cast<float> (getWidth()), static_cast<float> (getHeight()));
        }
    }

private:
    // ─── Visual particle state (persists across frames, smoothed) ───
    struct VisualParticle
    {
        float baseX       = 0.5f;   // target normalised X from grain data
        float baseY       = 0.5f;   // target normalised Y
        float displayAlpha = 0.0f;  // smoothed alpha (fade in/out)
        float displaySize  = 0.0f;  // smoothed size
        float driftPhaseX  = 0.0f;  // sinusoidal drift offset phase
        float driftPhaseY  = 0.0f;
        float driftSpeedX  = 0.4f;
        float driftSpeedY  = 0.3f;
        float hueOffset    = 0.0f;  // per-particle colour tint
        float envelope     = 0.0f;
        float pitch        = 0.0f;
        float pan          = 0.0f;
        float grainSize    = 0.0f;
        bool  wasActive    = false;
    };

    void timerCallback() override
    {
        currentData = latestData.load();

        // Advance global time (~22ms per tick at 45 Hz)
        const float dt = 1.0f / 45.0f;
        globalTime += dt;

        // Smooth each visual particle towards its grain data
        for (int i = 0; i < GranularConstants::kMaxGrains; ++i)
        {
            auto& vp = particles[static_cast<size_t> (i)];
            const auto& gi = currentData.grains[static_cast<size_t> (i)];

            // Advance drift phases
            vp.driftPhaseX += vp.driftSpeedX * dt;
            vp.driftPhaseY += vp.driftSpeedY * dt;

            if (gi.active)
            {
                // Map grain data to visual position
                // X: use grain's normalised position but as a STABLE scatter coordinate
                //    (avoid left-to-right sweep by using a hash-like mapping)
                const float stableX = hashPosition (gi.normPosition, static_cast<float> (i));
                // Y: pitch maps to vertical, with some envelope breathing
                const float stableY = 0.5f - gi.pitch / 48.0f;

                // Smooth towards target (exponential interpolation)
                const float posSmooth = gi.active && !vp.wasActive ? 0.6f : 0.08f; // snap faster on spawn
                vp.baseX = vp.wasActive ? lerp (vp.baseX, stableX, posSmooth) : stableX;
                vp.baseY = vp.wasActive ? lerp (vp.baseY, stableY, posSmooth) : stableY;
                vp.envelope = gi.envelope;
                vp.pitch    = gi.pitch;
                vp.pan      = gi.pan;
                vp.grainSize = gi.size;

                // Fade IN — gentle
                const float targetAlpha = gi.envelope;
                vp.displayAlpha = lerp (vp.displayAlpha, targetAlpha, 0.15f);

                const float targetSize = 2.0f + gi.size * 6.0f;
                vp.displaySize = lerp (vp.displaySize, targetSize, 0.12f);

                vp.wasActive = true;
            }
            else
            {
                // Fade OUT — slow and smooth (never abruptly disappear)
                vp.displayAlpha *= 0.92f;  // exponential decay over ~0.5s
                vp.displaySize  *= 0.95f;

                if (vp.displayAlpha < 0.005f)
                {
                    vp.displayAlpha = 0.0f;
                    vp.displaySize  = 0.0f;
                    vp.wasActive = false;
                }
            }
        }

        if (openGLContext.isAttached())
            openGLContext.triggerRepaint();
        else
            repaint();
    }

    void drawContent (juce::Graphics& g, float w, float h)
    {
        const float padding = 8.0f;
        const float areaX = padding;
        const float areaY = padding;
        const float areaW = w - padding * 2.0f;
        const float areaH = h - padding * 2.0f;

        // ── Background gradient (subtle vignette) ──
        {
            juce::ColourGradient vignette (
                Theme::background.brighter (0.04f), w * 0.5f, h * 0.5f,
                Theme::background.darker (0.15f),   0.0f, 0.0f, true);
            g.setGradientFill (vignette);
            g.fillRect (0.0f, 0.0f, w, h);
        }

        // ── Subtle grid ──
        drawGrid (g, areaX, areaY, areaW, areaH);

        // ── Draw ambient background glow (large, diffuse) ──
        drawAmbientGlow (g, areaX, areaY, areaW, areaH);

        // ── Draw all visual particles ──
        for (int i = 0; i < GranularConstants::kMaxGrains; ++i)
        {
            const auto& vp = particles[static_cast<size_t> (i)];
            if (vp.displayAlpha < 0.005f) continue;

            // Compute final position with gentle sinusoidal drift
            const float driftX = std::sin (vp.driftPhaseX) * 8.0f;  // ±8 pixels drift
            const float driftY = std::sin (vp.driftPhaseY) * 5.0f;  // ±5 pixels drift

            const float px = areaX + vp.baseX * areaW + driftX;
            const float py = areaY + vp.baseY * areaH + driftY;

            // Compute colour — rich palette based on pitch, pan, and per-particle offset
            auto colour = getParticleColour (vp.pan, vp.pitch, vp.hueOffset);

            const float alpha = vp.displayAlpha;
            const float sz = vp.displaySize;

            // Layer 1: Large soft outer glow (very faint, atmospheric)
            {
                const float glowR = sz * 4.0f;
                g.setColour (colour.withAlpha (alpha * 0.06f));
                g.fillEllipse (px - glowR, py - glowR, glowR * 2.0f, glowR * 2.0f);
            }

            // Layer 2: Medium glow
            {
                const float glowR = sz * 2.2f;
                g.setColour (colour.withAlpha (alpha * 0.12f));
                g.fillEllipse (px - glowR, py - glowR, glowR * 2.0f, glowR * 2.0f);
            }

            // Layer 3: Core body
            {
                const float coreR = sz * 0.8f;
                // Use a lighter tint for the core
                auto coreColour = colour.interpolatedWith (juce::Colours::white, 0.2f);
                g.setColour (coreColour.withAlpha (alpha * 0.55f));
                g.fillEllipse (px - coreR, py - coreR, coreR * 2.0f, coreR * 2.0f);
            }

            // Layer 4: Bright hot centre
            {
                const float centreR = sz * 0.3f;
                auto centreColour = colour.interpolatedWith (juce::Colours::white, 0.6f);
                g.setColour (centreColour.withAlpha (alpha * 0.85f));
                g.fillEllipse (px - centreR, py - centreR, centreR * 2.0f, centreR * 2.0f);
            }
        }

        // ── Info overlay ──
        g.setColour (Theme::textDim.withAlpha (0.6f));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText (juce::String (currentData.activeCount) + " grains",
                    juce::Rectangle<float> (w - 80.0f, h - 18.0f, 70.0f, 14.0f),
                    juce::Justification::centredRight);
    }

    // ── Ambient background glow: a very soft wash near the centre of particle mass ──
    void drawAmbientGlow (juce::Graphics& g, float areaX, float areaY, float areaW, float areaH)
    {
        if (currentData.activeCount == 0) return;

        // Compute centroid of active particles
        float cx = 0.0f, cy = 0.0f;
        int count = 0;

        for (int i = 0; i < GranularConstants::kMaxGrains; ++i)
        {
            const auto& vp = particles[static_cast<size_t> (i)];
            if (vp.displayAlpha < 0.02f) continue;
            cx += vp.baseX;
            cy += vp.baseY;
            ++count;
        }

        if (count == 0) return;
        cx /= static_cast<float> (count);
        cy /= static_cast<float> (count);

        const float gx = areaX + cx * areaW;
        const float gy = areaY + cy * areaH;
        const float gradRadius = std::min (areaW, areaH) * 0.5f;

        // Intensity based on active grain count (more grains = slightly brighter ambient)
        const float intensity = juce::jlimit (0.0f, 0.08f,
            static_cast<float> (count) / static_cast<float> (GranularConstants::kMaxGrains) * 0.12f);

        juce::ColourGradient ambientGrad (
            Theme::primaryCyan.withAlpha (intensity), gx, gy,
            Theme::primaryPurple.withAlpha (0.0f), gx + gradRadius, gy + gradRadius, true);
        g.setGradientFill (ambientGrad);
        g.fillEllipse (gx - gradRadius, gy - gradRadius, gradRadius * 2.0f, gradRadius * 2.0f);
    }

    void drawGrid (juce::Graphics& g, float x, float y, float w, float h)
    {
        // Very faint dots instead of harsh lines
        g.setColour (Theme::panelBorder.withAlpha (0.08f));

        // Dot grid every ~50 pixels
        const float spacing = 50.0f;
        for (float gx = x; gx <= x + w; gx += spacing)
        {
            for (float gy = y; gy <= y + h; gy += spacing)
            {
                g.fillEllipse (gx - 0.8f, gy - 0.8f, 1.6f, 1.6f);
            }
        }

        // Horizontal centre line (zero pitch)
        const float centreY = y + h * 0.5f;
        g.setColour (Theme::panelBorder.withAlpha (0.12f));
        const float dashLen = 4.0f;
        const float gapLen  = 6.0f;
        for (float dx = x; dx < x + w; dx += dashLen + gapLen)
            g.drawHorizontalLine (static_cast<int> (centreY), dx, std::min (dx + dashLen, x + w));
    }

    // ── Colour mapping: rich multi-hue palette ──
    static juce::Colour getParticleColour (float pan, float pitch, float hueOffset)
    {
        // Base hue cycles based on pitch: low pitch = warm (amber/red), mid = cyan, high = violet/pink
        // pan shifts the saturation/warmth
        const float pitchNorm = juce::jlimit (0.0f, 1.0f, (pitch + 24.0f) / 48.0f); // 0 = -24st, 1 = +24st
        const float panNorm   = (pan + 1.0f) * 0.5f; // 0 = left, 1 = right

        // Generate hue from pitch (0..1 → warm..cool..violet)
        float hue = 0.0f;
        if (pitchNorm < 0.33f)
        {
            // Low pitch → amber (hue ~0.08) to cyan (hue ~0.5)
            const float t = pitchNorm / 0.33f;
            hue = lerp (0.08f, 0.5f, t);
        }
        else if (pitchNorm < 0.66f)
        {
            // Mid pitch → cyan (0.5) to blue-purple (0.72)
            const float t = (pitchNorm - 0.33f) / 0.33f;
            hue = lerp (0.5f, 0.72f, t);
        }
        else
        {
            // High pitch → purple (0.72) to pink/magenta (0.88)
            const float t = (pitchNorm - 0.66f) / 0.34f;
            hue = lerp (0.72f, 0.88f, t);
        }

        // Add per-particle hue offset for variety
        hue = std::fmod (hue + hueOffset + 1.0f, 1.0f);

        // Saturation: left-panned = slightly desaturated, right = vivid
        const float sat = lerp (0.5f, 0.9f, panNorm);
        // Brightness: always bright
        const float bri = lerp (0.85f, 1.0f, panNorm);

        return juce::Colour::fromHSV (hue, sat, bri, 1.0f);
    }

    // ── Hash-like position mapping to avoid linear sweep ──
    // Maps normPosition (0..1) into a pseudo-random but stable X coordinate
    static float hashPosition (float normPos, float grainIndex)
    {
        // Use a combination of the position and grain index to create a
        // spatially distributed placement (instead of linear left→right)
        const float a = normPos * 7.919f + grainIndex * 0.618034f; // golden ratio for spacing
        const float frac = a - std::floor (a);
        // Smooth it with a subtle sinusoidal warp for visual appeal
        return 0.05f + 0.9f * (0.5f + 0.5f * std::sin (frac * juce::MathConstants<float>::twoPi));
    }

    static float lerp (float a, float b, float t) { return a + (b - a) * t; }

    juce::OpenGLContext openGLContext;
    std::atomic<GrainVisualData> latestData;
    GrainVisualData currentData;

    std::array<VisualParticle, GranularConstants::kMaxGrains> particles;
    float globalTime = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParticleVisualizer)
};
