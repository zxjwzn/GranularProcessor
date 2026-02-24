/*
  ==============================================================================
    PluginEditor.cpp
    Granular Processor — Main editor layout and logic.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

GranularProcessorAudioProcessorEditor::GranularProcessorAudioProcessorEditor (GranularProcessorAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      presetBar (p.getAPVTS())
{
    setLookAndFeel (&customLnf);

    // Window setup: scalable with fixed aspect ratio
    const int baseW = GranularConstants::kBaseWidth;
    const int baseH = GranularConstants::kBaseHeight;
    setResizable (true, true);
    setResizeLimits (static_cast<int> (baseW * GranularConstants::kMinScaleFactor),
                     static_cast<int> (baseH * GranularConstants::kMinScaleFactor),
                     static_cast<int> (baseW * GranularConstants::kMaxScaleFactor),
                     static_cast<int> (baseH * GranularConstants::kMaxScaleFactor));
    getConstrainer()->setFixedAspectRatio (static_cast<double> (baseW) / static_cast<double> (baseH));
    setSize (baseW, baseH);

    // Add top bar
    addAndMakeVisible (presetBar);

    // Add visualizer
    addAndMakeVisible (visualizer);

    // Add section panels
    addAndMakeVisible (grainPanel);
    addAndMakeVisible (scatterPanel);
    addAndMakeVisible (envelopePanel);
    addAndMakeVisible (effectsPanel);
    addAndMakeVisible (modulationPanel);
    addAndMakeVisible (outputPanel);
    addAndMakeVisible (controlPanel);

    // --- Grain knobs ---
    auto& apvts = audioProcessor.getAPVTS();

    grainPanel.addAndMakeVisible (knobGrainSize);
    grainPanel.addAndMakeVisible (knobDensity);
    grainPanel.addAndMakeVisible (knobPosition);
    grainPanel.addAndMakeVisible (knobPitch);
    grainPanel.addAndMakeVisible (knobPan);
    knobGrainSize.attachToParameter (apvts, ParamIDs::grainSize);
    knobDensity.attachToParameter (apvts, ParamIDs::grainDensity);
    knobPosition.attachToParameter (apvts, ParamIDs::grainPosition);
    knobPitch.attachToParameter (apvts, ParamIDs::grainPitch);
    knobPan.attachToParameter (apvts, ParamIDs::grainPan);

    // --- Scatter knobs ---
    scatterPanel.addAndMakeVisible (knobPosScatter);
    scatterPanel.addAndMakeVisible (knobPitchScatter);
    scatterPanel.addAndMakeVisible (knobPanScatter);
    knobPosScatter.attachToParameter (apvts, ParamIDs::posScatter);
    knobPitchScatter.attachToParameter (apvts, ParamIDs::pitchScatter);
    knobPanScatter.attachToParameter (apvts, ParamIDs::panScatter);

    // --- Envelope ---
    envelopePanel.addAndMakeVisible (knobAttack);
    envelopePanel.addAndMakeVisible (knobDecay);
    knobAttack.attachToParameter (apvts, ParamIDs::grainAttack);
    knobDecay.attachToParameter (apvts, ParamIDs::grainDecay);

    comboEnvShape.addItemList ({ "Hanning", "Gaussian", "Triangle", "Trapezoid" }, 1);
    envelopePanel.addAndMakeVisible (comboEnvShape);
    envShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::envelopeShape, comboEnvShape);

    // --- Effects ---
    effectsPanel.addAndMakeVisible (knobFeedback);
    effectsPanel.addAndMakeVisible (knobShimmer);
    effectsPanel.addAndMakeVisible (knobLowCut);
    effectsPanel.addAndMakeVisible (knobHighCut);
    knobFeedback.attachToParameter (apvts, ParamIDs::feedback);
    knobShimmer.attachToParameter (apvts, ParamIDs::shimmer);
    knobLowCut.attachToParameter (apvts, ParamIDs::lowCut);
    knobHighCut.attachToParameter (apvts, ParamIDs::highCut);

    // --- Modulation ---
    modulationPanel.addAndMakeVisible (knobLFORate);
    modulationPanel.addAndMakeVisible (knobLFODepth);
    knobLFORate.attachToParameter (apvts, ParamIDs::lfoRate);
    knobLFODepth.attachToParameter (apvts, ParamIDs::lfoDepth);

    comboLFOShape.addItemList ({ "Sine", "Triangle", "Square", "S&H" }, 1);
    modulationPanel.addAndMakeVisible (comboLFOShape);
    lfoShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::lfoShape, comboLFOShape);

    comboLFOTarget.addItemList ({ "Size", "Position", "Pitch", "Pan", "Filter" }, 1);
    modulationPanel.addAndMakeVisible (comboLFOTarget);
    lfoTargetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::lfoTarget, comboLFOTarget);

    // --- Output ---
    outputPanel.addAndMakeVisible (knobWidth);
    outputPanel.addAndMakeVisible (knobLevel);
    outputPanel.addAndMakeVisible (knobMix);
    outputPanel.addAndMakeVisible (knobBuffer);
    knobWidth.attachToParameter (apvts, ParamIDs::stereoWidth);
    knobLevel.attachToParameter (apvts, ParamIDs::outputLevel);
    knobMix.attachToParameter (apvts, ParamIDs::dryWet);
    knobBuffer.attachToParameter (apvts, ParamIDs::bufferLength);

    // --- Control buttons ---
    controlPanel.addAndMakeVisible (btnFreeze);
    controlPanel.addAndMakeVisible (btnReverse);
    btnFreeze.attachToParameter (apvts, ParamIDs::freeze);
    btnReverse.attachToParameter (apvts, ParamIDs::reverse);

    // Start timer for visualizer updates
    startTimerHz (30);
}

GranularProcessorAudioProcessorEditor::~GranularProcessorAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void GranularProcessorAudioProcessorEditor::timerCallback()
{
    visualizer.updateGrainData (audioProcessor.getGranularEngine().getVisualData());
}

void GranularProcessorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::background);
}

void GranularProcessorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    const int margin = 6;

    // 1) Top bar
    presetBar.setBounds (bounds.removeFromTop (40));

    // 2) Visualizer (~ 38% of remaining height)
    const int vizHeight = static_cast<int> ((bounds.getHeight()) * 0.38f);
    visualizer.setBounds (bounds.removeFromTop (vizHeight).reduced (margin, margin / 2));

    // 3) Middle row: Grain | Scatter | Envelope | Effects
    const int middleRowHeight = static_cast<int> ((bounds.getHeight()) * 0.52f);
    auto middleRow = bounds.removeFromTop (middleRowHeight).reduced (margin, margin / 2);
    {
        const int panelW = middleRow.getWidth() / 4;
        grainPanel.setBounds (middleRow.removeFromLeft (panelW).reduced (2));
        scatterPanel.setBounds (middleRow.removeFromLeft (panelW).reduced (2));
        envelopePanel.setBounds (middleRow.removeFromLeft (panelW).reduced (2));
        effectsPanel.setBounds (middleRow.reduced (2));
    }

    // 4) Bottom row: Modulation | Output | Control
    auto bottomRow = bounds.reduced (margin, margin / 2);
    {
        const int modW = static_cast<int> (bottomRow.getWidth() * 0.40f);
        const int outW = static_cast<int> (bottomRow.getWidth() * 0.35f);
        modulationPanel.setBounds (bottomRow.removeFromLeft (modW).reduced (2));
        outputPanel.setBounds (bottomRow.removeFromLeft (outW).reduced (2));
        controlPanel.setBounds (bottomRow.reduced (2));
    }

    // --- Layout knobs inside panels ---
    // Grain panel
    {
        auto area = grainPanel.getContentArea();
        const int knobW = area.getWidth() / 5;
        knobGrainSize.setBounds (area.removeFromLeft (knobW));
        knobDensity.setBounds (area.removeFromLeft (knobW));
        knobPosition.setBounds (area.removeFromLeft (knobW));
        knobPitch.setBounds (area.removeFromLeft (knobW));
        knobPan.setBounds (area);
    }

    // Scatter panel
    {
        auto area = scatterPanel.getContentArea();
        const int knobW = area.getWidth() / 3;
        knobPosScatter.setBounds (area.removeFromLeft (knobW));
        knobPitchScatter.setBounds (area.removeFromLeft (knobW));
        knobPanScatter.setBounds (area);
    }

    // Envelope panel
    {
        auto area = envelopePanel.getContentArea();
        const int knobW = area.getWidth() / 3;
        knobAttack.setBounds (area.removeFromLeft (knobW));
        knobDecay.setBounds (area.removeFromLeft (knobW));
        // Shape combo in remaining space
        auto comboArea = area.reduced (4);
        comboEnvShape.setBounds (comboArea.withHeight (24).withCentre (comboArea.getCentre()));
    }

    // Effects panel
    {
        auto area = effectsPanel.getContentArea();
        const int knobW = area.getWidth() / 4;
        knobFeedback.setBounds (area.removeFromLeft (knobW));
        knobShimmer.setBounds (area.removeFromLeft (knobW));
        knobLowCut.setBounds (area.removeFromLeft (knobW));
        knobHighCut.setBounds (area);
    }

    // Modulation panel
    {
        auto area = modulationPanel.getContentArea();
        const int knobW = area.getWidth() / 4;
        knobLFORate.setBounds (area.removeFromLeft (knobW));
        knobLFODepth.setBounds (area.removeFromLeft (knobW));
        auto comboArea = area;
        const int comboH = 22;
        const int gap = 4;
        comboLFOShape.setBounds (comboArea.removeFromTop (comboH + gap).removeFromTop (comboH).reduced (4, 0));
        comboLFOTarget.setBounds (comboArea.removeFromTop (comboH + gap).removeFromTop (comboH).reduced (4, 0));
    }

    // Output panel
    {
        auto area = outputPanel.getContentArea();
        const int knobW = area.getWidth() / 4;
        knobWidth.setBounds (area.removeFromLeft (knobW));
        knobLevel.setBounds (area.removeFromLeft (knobW));
        knobMix.setBounds (area.removeFromLeft (knobW));
        knobBuffer.setBounds (area);
    }

    // Control panel
    {
        auto area = controlPanel.getContentArea();
        const int btnH = (area.getHeight() - 8) / 2;
        btnFreeze.setBounds (area.removeFromTop (btnH).reduced (4, 2));
        area.removeFromTop (4);
        btnReverse.setBounds (area.removeFromTop (btnH).reduced (4, 2));
    }
}
