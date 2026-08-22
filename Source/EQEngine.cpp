#include "EQEngine.h"

#include <complex>

namespace eqmxa
{

EQEngine::EQEngine() = default;

void EQEngine::prepare (double newSampleRate, int /*blockSize*/, int numChannels)
{
    sampleRate = newSampleRate;
    numCh = juce::jlimit (1, kMaxCh, numChannels);
    reset();
}

void EQEngine::reset()
{
    for (auto& chStates : states)
        for (auto& s : chStates)
            s.reset();

    sLfBoost = lfBoostDb;
    sLfCut   = lfCutDb;
    sHfFreq  = hfFreqHz;
    sHfBoost = hfBoostDb;
    sHfCut   = hfCutDb;
    sectionsFor (sampleRate, lfIdx, sLfBoost, sLfCut, sHfFreq, sHfBoost, sHfCut, coeffs);
}

void EQEngine::setLfIndex (int idx)       { lfIdx     = juce::jlimit (0, 2, idx); }
void EQEngine::setLfBoostDb (float db)    { lfBoostDb = juce::jlimit (0.0f, 14.0f, db); }
void EQEngine::setLfCutDb (float db)      { lfCutDb   = juce::jlimit (0.0f, 16.0f, db); }
void EQEngine::setHfFreqHz (float hz)     { hfFreqHz  = juce::jlimit (3000.0f, 16000.0f, hz); }
void EQEngine::setHfBoostDb (float db)    { hfBoostDb = juce::jlimit (0.0f, 16.0f, db); }
void EQEngine::setHfCutDb (float db)      { hfCutDb   = juce::jlimit (0.0f, 16.0f, db); }

EQEngine::Coeffs EQEngine::lowShelf (double fs, float f0, float gainDb) noexcept
{
    const float A  = std::pow (10.0f, gainDb / 40.0f);
    const float w  = juce::MathConstants<float>::twoPi * f0 / (float) fs;
    const float cw = std::cos (w), sw = std::sin (w);
    const float al = sw * 0.7071f;   // pente de shelf S = 1
    const float sA = 2.0f * std::sqrt (A) * al;

    const float a0 = (A + 1.0f) + (A - 1.0f) * cw + sA;
    Coeffs c;
    c.b0 = A * ((A + 1.0f) - (A - 1.0f) * cw + sA) / a0;
    c.b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cw) / a0;
    c.b2 = A * ((A + 1.0f) - (A - 1.0f) * cw - sA) / a0;
    c.a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cw) / a0;
    c.a2 = ((A + 1.0f) + (A - 1.0f) * cw - sA) / a0;
    return c;
}

EQEngine::Coeffs EQEngine::highShelf (double fs, float f0, float gainDb) noexcept
{
    const float A  = std::pow (10.0f, gainDb / 40.0f);
    const float w  = juce::MathConstants<float>::twoPi * f0 / (float) fs;
    const float cw = std::cos (w), sw = std::sin (w);
    const float al = sw * 0.7071f;   // pente de shelf S = 1
    const float sA = 2.0f * std::sqrt (A) * al;

    const float a0 = (A + 1.0f) - (A - 1.0f) * cw + sA;
    Coeffs c;
    c.b0 = A * ((A + 1.0f) + (A - 1.0f) * cw + sA) / a0;
    c.b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cw) / a0;
    c.b2 = A * ((A + 1.0f) + (A - 1.0f) * cw - sA) / a0;
    c.a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cw) / a0;
    c.a2 = ((A + 1.0f) - (A - 1.0f) * cw - sA) / a0;
    return c;
}

EQEngine::Coeffs EQEngine::peaking (double fs, float f0, float q, float gainDb) noexcept
{
    const float A  = std::pow (10.0f, gainDb / 40.0f);
    const float w  = juce::MathConstants<float>::twoPi * f0 / (float) fs;
    const float cw = std::cos (w), sw = std::sin (w);
    const float al = sw / (2.0f * q);

    const float a0 = 1.0f + al / A;
    Coeffs c;
    c.b0 = (1.0f + al * A) / a0;
    c.b1 = -2.0f * cw / a0;
    c.b2 = (1.0f - al * A) / a0;
    c.a1 = -2.0f * cw / a0;
    c.a2 = (1.0f - al / A) / a0;
    return c;
}

float EQEngine::responseDb (float freqHz, double fs, const Coeffs sections[kNumSections]) noexcept
{
    const float w = juce::MathConstants<float>::twoPi * freqHz / (float) fs;
    const std::complex<float> z1 = std::polar (1.0f, -w);
    const std::complex<float> z2 = z1 * z1;

    std::complex<float> h { 1.0f, 0.0f };
    for (int i = 0; i < kNumSections; ++i)
    {
        const auto& c = sections[i];
        h *= (c.b0 + c.b1 * z1 + c.b2 * z2) / (1.0f + c.a1 * z1 + c.a2 * z2);
    }
    return 20.0f * std::log10 (std::abs (h) + 1.0e-9f);
}

void EQEngine::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int chs        = juce::jmin (numCh, buffer.getNumChannels());

    // Un pas de lissage par bloc : les gains rejoignent leur cible en douceur,
    // les coefficients sont recalcules une fois par bloc.
    const float k = 0.35f;
    sLfBoost += k * (lfBoostDb - sLfBoost);
    sLfCut   += k * (lfCutDb   - sLfCut);
    sHfFreq  += k * (hfFreqHz  - sHfFreq);
    sHfBoost += k * (hfBoostDb - sHfBoost);
    sHfCut   += k * (hfCutDb   - sHfCut);
    sectionsFor (sampleRate, lfIdx, sLfBoost, sLfCut, sHfFreq, sHfBoost, sHfCut, coeffs);

    for (int ch = 0; ch < chs; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int n = 0; n < numSamples; ++n)
        {
            float v = data[n];
            for (int i = 0; i < kNumSections; ++i)
                v = states[ch][i].process (v, coeffs[i]);
            data[n] = v;
        }
    }
}

}
