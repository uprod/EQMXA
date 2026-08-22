#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace eqmxa
{

// Coeur DSP de l'egaliseur de programme, dans l'esprit des EQ passifs a
// selecteur : une paire grave BOOST + ATTEN sur la meme frequence — mais
// l'attenuation agit un peu plus haut (x1.6) que le boost, c'est ce decalage
// qui creuse le fameux "dip" quand on pousse les deux —, une cloche haute
// large, et un shelf de coupure haute. Quatre biquads (formules RBJ) en
// cascade, memes coefficients pour les deux canaux.
class EQEngine
{
public:
    static constexpr int   kMaxCh       = 2;
    static constexpr int   kNumSections = 4;
    static constexpr float kAttenRatio  = 1.6f;   // l'ATTEN grave agit plus haut
    static constexpr float kHfQ         = 0.7f;   // cloche haute large

    struct Coeffs { float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f; };

    EQEngine();

    void prepare (double sampleRate, int blockSize, int numChannels);
    void reset();

    void setLfIndex (int idx);          // 0/1/2 -> 30/60/100 Hz
    void setLfBoostDb (float db);       // 0..+14
    void setLfCutDb (float db);         // 0..16 (applique en negatif)
    void setHfFreqHz (float hz);        // 3k..16k
    void setHfBoostDb (float db);       // 0..+16
    void setHfCutDb (float db);         // 0..16 (applique en negatif)

    // Traite le buffer en place.
    void process (juce::AudioBuffer<float>& buffer);

    // --- Verites partagees avec l'UI (FIG. 1 / FIG. 2) ----------------------
    static float lfFreqFor (int idx) noexcept
    {
        static constexpr float f[3] = { 30.0f, 60.0f, 100.0f };
        return f[juce::jlimit (0, 2, idx)];
    }

    // Coefficients RBJ (Audio EQ Cookbook), pente de shelf S = 1.
    static Coeffs lowShelf (double fs, float f0, float gainDb) noexcept;
    static Coeffs highShelf (double fs, float f0, float gainDb) noexcept;
    static Coeffs peaking (double fs, float f0, float q, float gainDb) noexcept;

    // Les quatre sections dans l'ordre du circuit : boost grave, atten grave
    // (corner x1.6), cloche haute, shelf de coupure haute. Meme algebre pour
    // le moteur et pour la courbe de FIG. 1.
    static void sectionsFor (double fs, int lfIdx, float lfBoostDb, float lfCutDb,
                             float hfHz, float hfBoostDb, float hfCutDb,
                             Coeffs out[kNumSections]) noexcept
    {
        const float fLf = lfFreqFor (lfIdx);
        out[0] = lowShelf  (fs, fLf, lfBoostDb);
        out[1] = lowShelf  (fs, fLf * kAttenRatio, -lfCutDb);
        out[2] = peaking   (fs, hfHz, kHfQ, hfBoostDb);
        out[3] = highShelf (fs, hfHz, -hfCutDb);
    }

    // Module (dB) de la cascade a une frequence donnee.
    static float responseDb (float freqHz, double fs, const Coeffs sections[kNumSections]) noexcept;

private:
    struct BiquadState
    {
        float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

        inline float process (float x, const Coeffs& c) noexcept
        {
            const float y = c.b0 * x + c.b1 * x1 + c.b2 * x2 - c.a1 * y1 - c.a2 * y2;
            x2 = x1; x1 = x;
            y2 = y1; y1 = y;
            return y;
        }

        void reset() noexcept { x1 = x2 = y1 = y2 = 0.0f; }
    };

    double sampleRate = 44100.0;
    int    numCh = 2;

    int   lfIdx     = 1;
    float lfBoostDb = 0.0f;
    float lfCutDb   = 0.0f;
    float hfFreqHz  = 8000.0f;
    float hfBoostDb = 0.0f;
    float hfCutDb   = 0.0f;

    // Gains lisses (anti-zipper, un pas par bloc) et coefficients courants.
    float sLfBoost = 0.0f, sLfCut = 0.0f, sHfFreq = 8000.0f, sHfBoost = 0.0f, sHfCut = 0.0f;
    Coeffs coeffs[kNumSections];
    BiquadState states[kMaxCh][kNumSections];
};

}
