#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

namespace eqmxa
{

// FIG. 1 - Reponse en frequence composite, tracee comme la figure d'un manuel
// technique. Ce n'est pas une illustration : la courbe est le module exact de
// la cascade des quatre biquads du moteur (EQEngine::sectionsFor /
// responseDb), y compris le "dip" que creuse la paire boost + atten grave.
// Le repaint est pilote par le Timer de l'editeur (~30 Hz).
class ResponsePlot : public juce::Component
{
public:
    explicit ResponsePlot (EQProcessor&);

    void paint (juce::Graphics&) override;

private:
    EQProcessor& processor;

    std::atomic<float>* lf      = nullptr;
    std::atomic<float>* lfboost = nullptr;
    std::atomic<float>* lfcut   = nullptr;
    std::atomic<float>* hffreq  = nullptr;
    std::atomic<float>* hfboost = nullptr;
    std::atomic<float>* hfcut   = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResponsePlot)
};

}
