#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace eqmxa
{

namespace IDs
{
    constexpr auto lf      = "lf";
    constexpr auto lfboost = "lfboost";
    constexpr auto lfcut   = "lfcut";
    constexpr auto hffreq  = "hffreq";
    constexpr auto hfboost = "hfboost";
    constexpr auto hfcut   = "hfcut";
}

juce::AudioProcessorValueTreeState::ParameterLayout EQProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Affichages "tally" a la machine a ecrire, aussi bien dans l'editeur que
    // dans les lignes d'automation de l'hote.
    const auto boostAttr = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int)
            { return juce::String (v >= 0.05f ? "+" : "") + juce::String (v, 1) + " dB"; })
        .withValueFromStringFunction ([] (const juce::String& t) { return t.getFloatValue(); });

    const auto cutAttr = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int)
            { return juce::String (v >= 0.05f ? "-" : "") + juce::String (v, 1) + " dB"; })
        .withValueFromStringFunction ([] (const juce::String& t) { return std::abs (t.getFloatValue()); });

    const auto khzAttr = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int) { return juce::String (v / 1000.0f, 1) + " kHz"; })
        .withValueFromStringFunction ([] (const juce::String& t)
            { const float f = t.getFloatValue(); return f < 100.0f ? f * 1000.0f : f; });

    // Selecteur de frequence grave, comme sur les EQ passifs d'epoque.
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { IDs::lf, 1 }, "LF Frequency",
        juce::StringArray { "30 Hz", "60 Hz", "100 Hz" }, 1));

    params.push_back (std::make_unique<P> (juce::ParameterID { IDs::lfboost, 1 },
        "Low Boost", juce::NormalisableRange<float> (0.0f, 14.0f, 0.1f), 0.0f, boostAttr));

    // L'attenuation grave agit un peu plus haut que le boost : les pousser
    // ensemble creuse le "dip" caracteristique.
    params.push_back (std::make_unique<P> (juce::ParameterID { IDs::lfcut, 1 },
        "Low Atten", juce::NormalisableRange<float> (0.0f, 16.0f, 0.1f), 0.0f, cutAttr));

    params.push_back (std::make_unique<P> (juce::ParameterID { IDs::hffreq, 1 },
        "HF Frequency", juce::NormalisableRange<float> (3000.0f, 16000.0f, 10.0f, 0.5f), 8000.0f, khzAttr));

    params.push_back (std::make_unique<P> (juce::ParameterID { IDs::hfboost, 1 },
        "High Boost", juce::NormalisableRange<float> (0.0f, 16.0f, 0.1f), 0.0f, boostAttr));

    params.push_back (std::make_unique<P> (juce::ParameterID { IDs::hfcut, 1 },
        "High Cut", juce::NormalisableRange<float> (0.0f, 16.0f, 0.1f), 0.0f, cutAttr));

    return { params.begin(), params.end() };
}

EQProcessor::EQProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
}

EQProcessor::~EQProcessor() = default;

void EQProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    engine.reset();
}

void EQProcessor::releaseResources()
{
    engine.reset();
}

bool EQProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& main = layouts.getMainOutputChannelSet();
    if (main != juce::AudioChannelSet::mono() && main != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == main;
}

void EQProcessor::pushParameterUpdatesToEngine()
{
    engine.setLfIndex   (juce::roundToInt (apvts.getRawParameterValue (IDs::lf)->load()));
    engine.setLfBoostDb (apvts.getRawParameterValue (IDs::lfboost)->load());
    engine.setLfCutDb   (apvts.getRawParameterValue (IDs::lfcut)->load());
    engine.setHfFreqHz  (apvts.getRawParameterValue (IDs::hffreq)->load());
    engine.setHfBoostDb (apvts.getRawParameterValue (IDs::hfboost)->load());
    engine.setHfCutDb   (apvts.getRawParameterValue (IDs::hfcut)->load());
}

void EQProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    pushParameterUpdatesToEngine();

    // L'egaliseur agit en place : pas de copie de travail.
    engine.process (buffer);
}

juce::AudioProcessorEditor* EQProcessor::createEditor()
{
    return new EQEditor (*this);
}

void EQProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void EQProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

}

// Point d'entree du plugin JUCE — doit etre au niveau global.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new eqmxa::EQProcessor();
}
