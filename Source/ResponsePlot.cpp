#include "ResponsePlot.h"
#include "ManualStyle.h"
#include "EQEngine.h"

namespace eqmxa
{

namespace
{
    constexpr float kFreqMin = 20.0f;
    constexpr float kFreqMax = 20000.0f;
    constexpr float kDbTop   = 18.0f;
    constexpr float kDbBot   = -18.0f;
    constexpr int   kPoints  = 200;

    float xForFreq (juce::Rectangle<float> r, float f)
    {
        const float t = std::log (f / kFreqMin) / std::log (kFreqMax / kFreqMin);
        return r.getX() + t * r.getWidth();
    }

    float yForDb (juce::Rectangle<float> r, float db)
    {
        const float t = (kDbTop - db) / (kDbTop - kDbBot);
        return r.getY() + t * r.getHeight();
    }

    float freqForX (juce::Rectangle<float> r, float x)
    {
        const float t = (x - r.getX()) / r.getWidth();
        return kFreqMin * std::pow (kFreqMax / kFreqMin, t);
    }

    juce::String freqLabel (float f)
    {
        if (f >= 1000.0f)
            return juce::String (f / 1000.0f, 0) + "k";
        return juce::String ((int) f);
    }
}

ResponsePlot::ResponsePlot (EQProcessor& proc)
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

void ResponsePlot::paint (juce::Graphics& g)
{
    auto full = getLocalBounds().toFloat();
    auto caption = full.removeFromBottom (16.0f);
    auto box = full;

    double fs = processor.getSampleRate();
    if (fs <= 0.0)
        fs = 48000.0;

    const int   lfIdx    = juce::roundToInt (lf->load());
    const float lfBoostV = lfboost->load();
    const float lfCutV   = lfcut->load();
    const float hfFreqV  = hffreq->load();
    const float hfBoostV = hfboost->load();
    const float hfCutV   = hfcut->load();

    EQEngine::Coeffs sections[EQEngine::kNumSections];
    EQEngine::sectionsFor (fs, lfIdx, lfBoostV, lfCutV, hfFreqV, hfBoostV, hfCutV, sections);

    const float fLf = EQEngine::lfFreqFor (lfIdx);

    // --- Grille ------------------------------------------------------------
    g.setColour (palette::inkFaint);
    static const float gridFreqs[] = { 50.0f, 100.0f, 200.0f, 500.0f,
                                       1000.0f, 2000.0f, 5000.0f, 10000.0f };
    for (const float f : gridFreqs)
        g.drawVerticalLine ((int) xForFreq (box, f), box.getY() + 1.0f, box.getBottom() - 1.0f);

    static const float gridDbs[] = { 12.0f, 6.0f, 0.0f, -6.0f, -12.0f };
    for (const float db : gridDbs)
    {
        g.setColour (db == 0.0f ? palette::inkMid.withAlpha (0.65f) : palette::inkFaint);
        g.drawHorizontalLine ((int) yForDb (box, db), box.getX() + 1.0f, box.getRight() - 1.0f);
    }

    // Reperes des deux sections : triangles sur l'axe, aux vraies frequences.
    for (const float f : { fLf, hfFreqV })
    {
        if (f < kFreqMin || f > kFreqMax)
            continue;
        const float sx = xForFreq (box, f);
        juce::Path idx;
        idx.addTriangle (sx - 3.5f, box.getBottom() - 1.0f,
                         sx + 3.5f, box.getBottom() - 1.0f,
                         sx, box.getBottom() - 7.0f);
        g.setColour (palette::spot);
        g.fillPath (idx);
    }

    // --- Courbe composite ------------------------------------------------------
    {
        juce::Path p;
        for (int i = 0; i < kPoints; ++i)
        {
            const float px = box.getX() + (float) i / (float) (kPoints - 1) * box.getWidth();
            const float f  = freqForX (box, px);
            const float db = juce::jlimit (kDbBot, kDbTop, EQEngine::responseDb (f, fs, sections));
            const float py = yForDb (box, db);
            if (i == 0) p.startNewSubPath (px, py);
            else        p.lineTo (px, py);
        }
        g.setColour (palette::spot);
        g.strokePath (p, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved));
    }

    // --- Echelles ----------------------------------------------------------
    // Chaque chiffre est pose sur un cartouche film : ni le grain ni la grille
    // ne peuvent corrompre une valeur que le regard doit pouvoir croire.
    auto drawFigure = [&g] (const juce::String& text, juce::Point<float> anchor,
                            juce::Justification just)
    {
        const auto font = fonts::mono (9.0f);
        const float tw  = juce::GlyphArrangement::getStringWidth (font, text);
        auto area = juce::Rectangle<float> (tw + 6.0f, 11.0f).withCentre (anchor);
        if (just.testFlags (juce::Justification::left))
            area.setX (anchor.x - 3.0f);
        else if (just.testFlags (juce::Justification::right))
            area.setX (anchor.x - tw - 3.0f);

        g.setColour (palette::film);
        g.fillRect (area);
        g.setFont (font);
        g.setColour (palette::inkMid);
        g.drawText (text, area, juce::Justification::centred);
    };

    for (const float f : gridFreqs)
        drawFigure (freqLabel (f), { xForFreq (box, f), box.getBottom() - 8.0f },
                    juce::Justification::centred);

    for (const float db : gridDbs)
        drawFigure ((db > 0.0f ? "+" : "") + juce::String ((int) db),
                    { box.getX() + 6.0f, yForDb (box, db) - 6.0f },
                    juce::Justification::left);

    // Designation des unites, une fois par echelle, convention de plan.
    drawFigure ("dB", { box.getX() + 6.0f, box.getY() + 10.0f }, juce::Justification::left);
    drawFigure ("Hz", { box.getRight() - 6.0f, box.getBottom() - 8.0f }, juce::Justification::right);

    // Tally du "dip" : le creux reel entre le boost et l'attenuation grave.
    {
        float dipDb = 0.0f;
        for (float f = fLf; f <= fLf * 6.0f; f *= 1.06f)
            dipDb = juce::jmin (dipDb, EQEngine::responseDb (f, fs, sections));

        auto tally = juce::Rectangle<float> (120.0f, 12.0f)
                         .withPosition (box.getRight() - 126.0f, box.getY() + 6.0f);
        g.setColour (palette::film);
        g.fillRect (tally.expanded (3.0f, 1.0f));
        g.setFont (fonts::mono (10.0f));
        g.setColour (palette::inkMid);
        g.drawText ("dip", tally.removeFromLeft (30.0f), juce::Justification::centredLeft);
        g.setColour (palette::ink);
        g.drawText (juce::String (dipDb, 1) + " dB", tally, juce::Justification::centredLeft);
    }

    // --- Cadre + legende de figure ------------------------------------------
    g.setColour (palette::ink);
    g.drawRect (box, 1.0f);

    const juce::String cap = "FIG. 1 - COMPOSITE FREQUENCY RESPONSE, FOUR SECTIONS";
    g.setFont (fonts::lettering (10.0f));
    g.setColour (palette::inkMid);
    g.drawText (cap, caption.withTrimmedTop (4.0f), juce::Justification::bottomLeft);

    const float capW = juce::GlyphArrangement::getStringWidth (fonts::lettering (10.0f), cap);
    g.setColour (palette::inkFaint);
    g.drawHorizontalLine ((int) (caption.getBottom() - 3.0f),
                          caption.getX() + capW + 10.0f, caption.getRight());
}

}
