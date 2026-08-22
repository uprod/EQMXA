#include "SchematicDiagram.h"
#include "ManualStyle.h"
#include "EQEngine.h"

namespace eqmxa
{

namespace
{
    void drawArrowHead (juce::Graphics& g, juce::Point<float> tip, juce::Point<float> dir, float size)
    {
        dir = dir / (dir.getDistanceFromOrigin() + 1.0e-6f);
        const juce::Point<float> n (-dir.y, dir.x);
        juce::Path p;
        p.addTriangle (tip, tip - dir * size + n * (size * 0.55f),
                             tip - dir * size - n * (size * 0.55f));
        g.fillPath (p);
    }

    // Motif L-C des filtres passifs : bobine (spires) puis condensateur
    // (deux lames), purement notationnel, comme un rappel de la topologie.
    void drawLCMotif (juce::Graphics& g, float cx, float cy)
    {
        g.setColour (palette::inkMid);

        // Amorce, bobine de 4 spires, liaison, condensateur, amorce.
        const float x0 = cx - 46.0f;
        g.drawLine (x0, cy, x0 + 10.0f, cy, 0.9f);

        juce::Path coil;
        const int turns = 4, pts = turns * 10;
        for (int i = 0; i <= pts; ++i)
        {
            const float t  = (float) i / (float) pts;
            const float sx = x0 + 10.0f + 34.0f * t;
            const float sy = cy - 3.5f * std::abs (std::sin (t * (float) turns
                                                             * juce::MathConstants<float>::pi));
            if (i == 0) coil.startNewSubPath (sx, sy);
            else        coil.lineTo (sx, sy);
        }
        g.strokePath (coil, juce::PathStrokeType (0.9f));

        const float capX = x0 + 52.0f;
        g.drawLine (x0 + 44.0f, cy, capX, cy, 0.9f);
        g.drawLine (capX, cy - 5.0f, capX, cy + 5.0f, 1.1f);
        g.drawLine (capX + 4.0f, cy - 5.0f, capX + 4.0f, cy + 5.0f, 1.1f);
        g.drawLine (capX + 4.0f, cy, capX + 14.0f, cy, 0.9f);

        g.setFont (fonts::mono (8.0f));
        g.drawText ("L-C", juce::Rectangle<float> (26.0f, 10.0f).withPosition (capX + 18.0f, cy - 5.0f),
                    juce::Justification::centredLeft);
    }
}

SchematicDiagram::SchematicDiagram (EQProcessor& proc)
    : processor (proc)
{
    auto& apvts = processor.getAPVTS();
    lf      = apvts.getRawParameterValue ("lf");
    lfboost = apvts.getRawParameterValue ("lfboost");
    lfcut   = apvts.getRawParameterValue ("lfcut");
    hffreq  = apvts.getRawParameterValue ("hffreq");
    hfboost = apvts.getRawParameterValue ("hfboost");
    hfcut   = apvts.getRawParameterValue ("hfcut");

    setInterceptsMouseClicks (false, false);
}

void SchematicDiagram::paint (juce::Graphics& g)
{
    const float w = (float) getWidth();
    auto caption  = getLocalBounds().toFloat().removeFromBottom (16.0f);

    const int   lfIdx    = juce::roundToInt (lf->load());
    const float lfBoostV = lfboost->load();
    const float lfCutV   = lfcut->load();
    const float hfFreqV  = hffreq->load();
    const float hfBoostV = hfboost->load();
    const float hfCutV   = hfcut->load();

    const float fLf = EQEngine::lfFreqFor (lfIdx);

    // Rangs horizontaux du schema.
    const float railY  = 34.0f;
    const float motifY = 64.0f;

    // Colonnes.
    const float inX    = 12.0f;
    const float loX0   = 110.0f, loX1 = 300.0f;   // section grave
    const float hiX0   = 360.0f, hiX1 = 560.0f;   // section haute
    const float outX   = w - 16.0f;
    const float blockH = 30.0f;

    // --- Rail d'entree -----------------------------------------------------
    g.setColour (palette::ink);
    g.drawEllipse (inX - 3.0f, railY - 3.0f, 6.0f, 6.0f, 1.1f);              // borne IN
    g.drawLine (inX + 3.0f, railY, loX0, railY, 1.2f);
    drawArrowHead (g, { loX0, railY }, { 1.0f, 0.0f }, 6.0f);

    g.setFont (fonts::lettering (9.0f));
    g.setColour (palette::inkMid);
    g.drawText ("IN", juce::Rectangle<float> (24.0f, 10.0f).withPosition (inX - 8.0f, railY - 17.0f),
                juce::Justification::centredLeft);

    // --- Section grave : la paire boost / attenuation ------------------------
    {
        const juce::Rectangle<float> block (loX0, railY - blockH * 0.5f, loX1 - loX0, blockH);
        g.setColour (palette::film);
        g.fillRect (block);
        g.setColour (palette::ink);
        g.drawRect (block, 1.2f);

        g.setFont (fonts::lettering (10.0f));
        g.drawText ("LOW SHELF PAIR", block.withTrimmedBottom (10.0f), juce::Justification::centred);

        const juce::String sub = "+" + juce::String (lfBoostV, 1) + " / -"
            + juce::String (lfCutV, 1) + " dB @ " + juce::String ((int) fLf) + " Hz";
        g.setFont (fonts::mono (8.0f));
        g.setColour (palette::inkMid);
        g.drawText (sub, block.withTrimmedTop (13.0f), juce::Justification::centred);
    }

    // --- Section haute : cloche + shelf de coupure ----------------------------
    g.setColour (palette::ink);
    g.drawLine (loX1, railY, hiX0, railY, 1.2f);
    {
        const juce::Rectangle<float> block (hiX0, railY - blockH * 0.5f, hiX1 - hiX0, blockH);
        g.setColour (palette::film);
        g.fillRect (block);
        g.setColour (palette::ink);
        g.drawRect (block, 1.2f);

        g.setFont (fonts::lettering (10.0f));
        g.drawText ("HIGH SECTION", block.withTrimmedBottom (10.0f), juce::Justification::centred);

        const juce::String sub = "PK +" + juce::String (hfBoostV, 1) + " @ "
            + juce::String (hfFreqV / 1000.0f, 1) + "k / SH -" + juce::String (hfCutV, 1);
        g.setFont (fonts::mono (8.0f));
        g.setColour (palette::inkMid);
        g.drawText (sub, block.withTrimmedTop (13.0f), juce::Justification::centred);
    }

    // --- Sortie ----------------------------------------------------------------
    g.setColour (palette::ink);
    g.drawLine (hiX1, railY, outX - 3.0f, railY, 1.4f);
    g.fillEllipse (outX - 3.0f, railY - 3.0f, 6.0f, 6.0f);                   // borne OUT
    g.setColour (palette::inkMid);
    g.drawText ("OUT", juce::Rectangle<float> (28.0f, 10.0f).withPosition (outX - 24.0f, railY - 17.0f),
                juce::Justification::centredRight);

    // Motifs L-C sous chaque section : la topologie passive dont ce circuit
    // reprend l'esprit.
    drawLCMotif (g, (loX0 + loX1) * 0.5f, motifY);
    drawLCMotif (g, (hiX0 + hiX1) * 0.5f, motifY);

    // --- Legende de figure ----------------------------------------------------
    const juce::String cap = "FIG. 2 - SIGNAL PATH, PASSIVE-STYLE EQ SECTIONS";
    g.setFont (fonts::lettering (10.0f));
    g.setColour (palette::inkMid);
    g.drawText (cap, caption.withTrimmedTop (4.0f), juce::Justification::bottomLeft);

    const float capW = juce::GlyphArrangement::getStringWidth (fonts::lettering (10.0f), cap);
    g.setColour (palette::inkFaint);
    g.drawHorizontalLine ((int) (caption.getBottom() - 3.0f),
                          caption.getX() + capW + 10.0f, caption.getRight());
}

}
