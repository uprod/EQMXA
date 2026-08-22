#pragma once

/*  IMPECCABLE DIRECTION CONTRACT — seed 5bcea053 (roll: assigned)
    THESIS: The panel IS the signal path — a service-manual schematic read as
    the circuit you hear; refuses knobs-on-a-metal-plate.
    OWN-WORLD: Diazo film negative — dark drafting film #17140F, pale ink
    #E6DCC2, spot Prussian-blue #5C85E6 (one spot ink per MXA sibling).
    Routed Gothic drafting lettering + Courier Prime figures, double sheet
    border, title block, FIG. captions.
    STORY: A producer reads the schematic, watches the composite curve and
    the famous low-shelf dip take shape in FIG. 1, and trusts every figure
    at a glance.
    FIRST VIEWPORT: Header + title block; FIG. 1 exact composite response of
    the four cascaded sections full width; FIG. 2 signal path with the low
    shelf pair and high section, real dB figures printed, passive L-C motifs;
    six schematic dials beneath.
    SIGNATURE: the dip — FIG. 1's curve and its printed dip tally, driven by
    the engine's exact biquad algebra.
    FORM: Service Manual family template, adopted from PhaserMXA, seed 5bcea053.
    FINISH: unreviewed and undocumented is unfinished; this build ends with
    the finish review, the verdict, DESIGN.md, and every shipping raster
    carrying its provenance.
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ManualStyle.h"
#include "ResponsePlot.h"
#include "SchematicDiagram.h"

namespace eqmxa
{

class EQEditor : public juce::AudioProcessorEditor,
                 private juce::Timer
{
public:
    explicit EQEditor (EQProcessor& proc);
    ~EQEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent& e) override;

private:
    using APVTS   = juce::AudioProcessorValueTreeState;
    using SAttach = APVTS::SliderAttachment;

    struct Dial
    {
        juce::Slider slider;
        juce::Label  name;
        std::unique_ptr<SAttach> attachment;
    };

    void setupDial (Dial& d, const juce::String& labelText, const juce::String& paramID);
    void timerCallback() override;

    void drawSheetFrame (juce::Graphics& g);
    void drawHeader (juce::Graphics& g);

    EQProcessor& processor;

    ManualLookAndFeel lookAndFeel;
    juce::Image       filmTexture;

    ResponsePlot     plot;
    SchematicDiagram schematic;

    Dial lfSwitch, lfBoostDial, lfCutDial, hfFreqDial, hfBoostDial, hfCutDial;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQEditor)
};

}
