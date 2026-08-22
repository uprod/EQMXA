#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

namespace eqmxa
{

// FIG. 2 - Le chemin du signal dessine comme dans le manuel : IN, la paire de
// shelfs graves (boost et attenuation imprimes avec leurs vrais dB et la
// vraie frequence du selecteur), la section haute (cloche + shelf, vraies
// valeurs), et les motifs L-C des filtres passifs dont ce circuit reprend
// l'esprit. La quantite est imprimee en chiffres vrais : le schema est la
// valeur.
class SchematicDiagram : public juce::Component
{
public:
    explicit SchematicDiagram (EQProcessor&);

    void paint (juce::Graphics&) override;

private:
    EQProcessor& processor;

    std::atomic<float>* lf      = nullptr;
    std::atomic<float>* lfboost = nullptr;
    std::atomic<float>* lfcut   = nullptr;
    std::atomic<float>* hffreq  = nullptr;
    std::atomic<float>* hfboost = nullptr;
    std::atomic<float>* hfcut   = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SchematicDiagram)
};

}
