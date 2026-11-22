#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

// Eén lijst met alle parameter-ID's: enum en namen blijven automatisch gelijk.
// De mm*_src/dst/amt moeten per slot direct na elkaar staan (voice rekent met +3*k).
#define PARAM_LIST(X) \
    X(osc1_shape) X(osc1_semi) X(osc1_fine) X(osc1_pw) \
    X(osc2_shape) X(osc2_semi) X(osc2_fine) X(osc2_pw) \
    X(osc3_shape) X(osc3_semi) X(osc3_fine) X(osc3_pw) X(osc3_level) X(ring_level) \
    X(osc_mix) X(sub_level) X(sub_shape) X(noise_level) X(uni_voices) X(uni_spread) \
    X(flt_type) X(flt_slope) X(flt_cutoff) X(flt_res) X(flt_env) X(flt_key) \
    X(f2_type) X(f_route) X(f2_cutoff) X(f2_res) X(f2_env) X(f2_key) \
    X(fenv_a) X(fenv_d) X(fenv_s) X(fenv_r) \
    X(aenv_a) X(aenv_d) X(aenv_s) X(aenv_r) \
    X(lfo_rate) X(lfo_shape) X(lfo_pitch) X(lfo_cutoff) X(lfo2_rate) X(lfo2_shape) \
    X(mm1_src) X(mm1_dst) X(mm1_amt) X(mm2_src) X(mm2_dst) X(mm2_amt) \
    X(mm3_src) X(mm3_dst) X(mm3_amt) X(mm4_src) X(mm4_dst) X(mm4_amt) \
    X(arp_on) X(arp_mode) X(arp_rate) X(arp_oct) X(arp_gate) X(arp_sync) X(arp_bpm) \
    X(chorus_mix) X(reverb_mix) X(master)

namespace P
{
enum Id
{
#define X(n) n,
    PARAM_LIST(X)
#undef X
    count
};

inline const char* name (int i)
{
    static const char* names[] = {
#define X(n) #n,
        PARAM_LIST(X)
#undef X
    };
    return names[i];
}
} // namespace P

// Volgorde hier = volgorde van de index in SynthVoice::control()
inline juce::StringArray modSources()
{
    return { "Off", "LFO 1", "LFO 2", "Flt Env", "Amp Env", "Velocity", "Keytrack", "Mod Wheel", "Aftertouch", "Random" };
}

inline juce::StringArray modDests()
{
    return { "Off", "Pitch", "Osc1 Pitch", "Osc2 Pitch", "Osc3 Pitch", "Pulse Width",
             "Cutoff 1", "Cutoff 2", "Resonance", "Osc Mix", "Amp Level", "Pan" };
}

// Waarden die buiten de voices om binnenkomen (mod wheel, aftertouch)
struct SharedState
{
    std::atomic<float> modWheel { 0.0f }, aftertouch { 0.0f };
};
