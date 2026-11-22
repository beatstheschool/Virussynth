#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

AudioProcessorValueTreeState::ParameterLayout FrostSynthProcessor::createLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto flt = [&] (const String& id, const String& nm, float lo, float hi, float def, float skew = 1.0f)
    {
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm,
                                                            NormalisableRange<float> (lo, hi, 0.0f, skew), def));
    };
    auto integer = [&] (const String& id, const String& nm, int lo, int hi, int def)
    {
        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { id, 1 }, nm, lo, hi, def));
    };
    auto choice = [&] (const String& id, const String& nm, StringArray items, int def)
    {
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, nm, items, def));
    };
    auto toggle = [&] (const String& id, const String& nm, bool def)
    {
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, nm, def));
    };

    const StringArray shapes { "Saw", "Pulse", "Tri", "Sine" };

    choice  ("osc1_shape", "Osc1 Shape", shapes, 0);
    integer ("osc1_semi",  "Osc1 Semi", -24, 24, 0);
    flt     ("osc1_fine",  "Osc1 Fine", -50.0f, 50.0f, 0.0f);
    flt     ("osc1_pw",    "Osc1 PW", 0.05f, 0.95f, 0.5f);

    choice  ("osc2_shape", "Osc2 Shape", shapes, 0);
    integer ("osc2_semi",  "Osc2 Semi", -24, 24, 0);
    flt     ("osc2_fine",  "Osc2 Fine", -50.0f, 50.0f, 8.0f);
    flt     ("osc2_pw",    "Osc2 PW", 0.05f, 0.95f, 0.5f);

    choice  ("osc3_shape", "Osc3 Shape", shapes, 1);
    integer ("osc3_semi",  "Osc3 Semi", -24, 24, 0);
    flt     ("osc3_fine",  "Osc3 Fine", -50.0f, 50.0f, -8.0f);
    flt     ("osc3_pw",    "Osc3 PW", 0.05f, 0.95f, 0.5f);
    flt     ("osc3_level", "Osc3 Level", 0.0f, 1.0f, 0.0f);
    flt     ("ring_level", "Ring Mod", 0.0f, 1.0f, 0.0f);

    flt     ("osc_mix",    "Osc1/2 Mix", 0.0f, 1.0f, 0.5f);
    flt     ("sub_level",  "Sub", 0.0f, 1.0f, 0.0f);
    choice  ("sub_shape",  "Sub Shape", { "Square", "Sine" }, 1);
    flt     ("noise_level","Noise", 0.0f, 1.0f, 0.0f);
    integer ("uni_voices", "Unison", 1, 5, 3);
    flt     ("uni_spread", "Spread", 0.0f, 50.0f, 12.0f);

    choice  ("flt_type",   "Filter 1", { "LP", "HP", "BP" }, 0);
    choice  ("flt_slope",  "F1 Slope", { "12 dB", "24 dB" }, 1);
    flt     ("flt_cutoff", "F1 Cutoff", 20.0f, 20000.0f, 1800.0f, 0.3f);
    flt     ("flt_res",    "F1 Reso", 0.0f, 1.0f, 0.25f);
    flt     ("flt_env",    "F1 Env", -1.0f, 1.0f, 0.4f);
    flt     ("flt_key",    "F1 Key", 0.0f, 1.0f, 0.3f);

    choice  ("f2_type",    "Filter 2", { "Off", "LP", "HP", "BP" }, 0);
    choice  ("f_route",    "Routing", { "Serial", "Parallel" }, 0);
    flt     ("f2_cutoff",  "F2 Cutoff", 20.0f, 20000.0f, 1000.0f, 0.3f);
    flt     ("f2_res",     "F2 Reso", 0.0f, 1.0f, 0.2f);
    flt     ("f2_env",     "F2 Env", -1.0f, 1.0f, 0.0f);
    flt     ("f2_key",     "F2 Key", 0.0f, 1.0f, 0.0f);

    flt ("fenv_a", "Flt Atk", 0.001f, 5.0f, 0.005f, 0.4f);
    flt ("fenv_d", "Flt Dec", 0.001f, 5.0f, 0.5f,   0.4f);
    flt ("fenv_s", "Flt Sus", 0.0f,   1.0f, 0.3f);
    flt ("fenv_r", "Flt Rel", 0.001f, 8.0f, 0.4f,   0.4f);
    flt ("aenv_a", "Amp Atk", 0.001f, 5.0f, 0.005f, 0.4f);
    flt ("aenv_d", "Amp Dec", 0.001f, 5.0f, 0.3f,   0.4f);
    flt ("aenv_s", "Amp Sus", 0.0f,   1.0f, 0.8f);
    flt ("aenv_r", "Amp Rel", 0.001f, 8.0f, 0.3f,   0.4f);

    const StringArray lfoShapes { "Sine", "Tri", "Square", "Saw" };
    flt    ("lfo_rate",   "LFO1 Rate", 0.05f, 20.0f, 4.0f, 0.4f);
    choice ("lfo_shape",  "LFO1 Shape", lfoShapes, 0);
    flt    ("lfo_pitch",  "LFO1>Pitch", 0.0f, 12.0f, 0.0f);
    flt    ("lfo_cutoff", "LFO1>Cutoff", 0.0f, 1.0f, 0.0f);
    flt    ("lfo2_rate",  "LFO2 Rate", 0.05f, 20.0f, 0.5f, 0.4f);
    choice ("lfo2_shape", "LFO2 Shape", lfoShapes, 1);

    for (int k = 1; k <= 4; ++k)
    {
        const String n (k);
        choice ("mm" + n + "_src", "Mod " + n + " Src", modSources(), 0);
        choice ("mm" + n + "_dst", "Mod " + n + " Dst", modDests(), 0);
        flt    ("mm" + n + "_amt", "Mod " + n + " Amt", -1.0f, 1.0f, 0.0f);
    }

    toggle  ("arp_on",   "Arp", false);
    choice  ("arp_mode", "Arp Mode", { "Up", "Down", "UpDown", "Random", "As Played" }, 0);
    choice  ("arp_rate", "Arp Rate", { "1/4", "1/8", "1/16", "1/32" }, 2);
    integer ("arp_oct",  "Arp Octaves", 1, 4, 1);
    flt     ("arp_gate", "Arp Gate", 0.1f, 1.0f, 0.6f);
    choice  ("arp_sync", "Arp Tempo", { "Manual", "Host" }, 1);
    flt     ("arp_bpm",  "Arp BPM", 40.0f, 240.0f, 120.0f);

    flt ("chorus_mix", "Chorus", 0.0f, 1.0f, 0.3f);
    flt ("reverb_mix", "Reverb", 0.0f, 1.0f, 0.15f);
    flt ("master",     "Master", 0.0f, 1.0f, 0.7f);

    return { p.begin(), p.end() };
}

FrostSynthProcessor::FrostSynthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    synth.addSound (new SynthSound());
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new SynthVoice (apvts, shared));
}

void FrostSynthProcessor::prepareToPlay (double sr, int block)
{
    synth.setCurrentPlaybackSampleRate (sr);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            v->prepare (sr, block);

    arp.reset();

    chorus.prepare ({ sr, (uint32) block, 2 });
    chorus.setRate (0.8f);
    chorus.setDepth (0.25f);
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback (0.0f);

    reverb.setSampleRate (sr);
    reverb.reset();
}

bool FrostSynthProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == AudioChannelSet::stereo();
}

void FrostSynthProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    auto pv = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    // Mod wheel en aftertouch voor de modulatiematrix
    for (auto md : midi)
    {
        const auto m = md.getMessage();
        if (m.isController() && m.getControllerNumber() == 1)
            shared.modWheel = (float) m.getControllerValue() / 127.0f;
        else if (m.isChannelPressure())
            shared.aftertouch = (float) m.getChannelPressureValue() / 127.0f;
    }

    // Arpeggiator (tempo van de host, of handmatig)
    Arpeggiator::Settings as;
    as.on      = pv ("arp_on") > 0.5f;
    as.mode    = (int) pv ("arp_mode");
    as.rateIdx = (int) pv ("arp_rate");
    as.octaves = (int) pv ("arp_oct");
    as.gate    = pv ("arp_gate");
    as.bpm     = pv ("arp_bpm");
    if (pv ("arp_sync") > 0.5f)
        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
                if (auto bpm = pos->getBpm())
                    as.bpm = *bpm;
    arp.process (midi, n, getSampleRate(), as);

    buffer.clear();
    synth.renderNextBlock (buffer, midi, 0, n);

    chorus.setMix (pv ("chorus_mix"));
    dsp::AudioBlock<float> block (buffer);
    chorus.process (dsp::ProcessContextReplacing<float> (block));

    if (buffer.getNumChannels() >= 2)
    {
        const float rm = pv ("reverb_mix");
        Reverb::Parameters rp;
        rp.roomSize = 0.6f;
        rp.damping  = 0.5f;
        rp.width    = 1.0f;
        rp.wetLevel = rm * 0.5f;
        rp.dryLevel = 1.0f - rm * 0.3f;
        reverb.setParameters (rp);
        reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), n);
    }

    buffer.applyGain (pv ("master"));
}

void FrostSynthProcessor::setCurrentProgram (int index)
{
    const auto& presets = factoryPresets();
    if (index < 0 || index >= (int) presets.size())
        return;

    currentProgram = index;

    for (auto* par : getParameters())
        par->setValueNotifyingHost (par->getDefaultValue());

    for (auto& kv : presets[(size_t) index].values)
        if (auto* par = apvts.getParameter (kv.first))
            par->setValueNotifyingHost (par->convertTo0to1 (kv.second));
}

const String FrostSynthProcessor::getProgramName (int index)
{
    const auto& presets = factoryPresets();
    return index >= 0 && index < (int) presets.size() ? String (presets[(size_t) index].name) : String();
}

AudioProcessorEditor* FrostSynthProcessor::createEditor()
{
    return new FrostSynthEditor (*this);
}

void FrostSynthProcessor::getStateInformation (MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void FrostSynthProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (ValueTree::fromXml (*xml));
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FrostSynthProcessor();
}
