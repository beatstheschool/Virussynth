#pragma once
#include "Params.h"

// Waarden zijn echte parameterwaarden; keuzelijsten gebruiken de index:
// shape: Saw0 Pulse1 Tri2 Sine3 | flt_type: LP0 HP1 BP2 | f2_type: Off0 LP1 HP2 BP3
// lfo_shape: Sine0 Tri1 Square2 Saw3 | arp_mode: Up0 Down1 UpDown2 Random3 AsPlayed4
// mm src: 1 LFO1, 2 LFO2, 3 FltEnv, 4 AmpEnv, 5 Vel, 6 Key, 7 ModWheel, 8 AT, 9 Random
// mm dst: 1 Pitch, 2-4 Osc1-3 Pitch, 5 PW, 6 Cutoff1, 7 Cutoff2, 8 Reso, 9 Mix, 10 Amp, 11 Pan
struct Preset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;
};

inline const std::vector<Preset>& factoryPresets()
{
    static const std::vector<Preset> presets = {
        { "Init", {} },

        { "Supersaw Lead", {
            { "uni_voices", 5 }, { "uni_spread", 30.0f }, { "osc2_fine", 6.0f },
            { "flt_cutoff", 5000.0f }, { "flt_res", 0.15f }, { "flt_env", 0.2f },
            { "aenv_a", 0.01f }, { "aenv_r", 0.5f }, { "chorus_mix", 0.4f }, { "reverb_mix", 0.25f },
            { "mm1_src", 7 }, { "mm1_dst", 6 }, { "mm1_amt", 0.5f } } },

        { "Fat Bass", {
            { "osc2_shape", 1 }, { "osc2_semi", -12 }, { "uni_voices", 1 }, { "uni_spread", 0.0f },
            { "sub_level", 0.6f }, { "flt_cutoff", 300.0f }, { "flt_res", 0.3f }, { "flt_env", 0.6f },
            { "fenv_d", 0.25f }, { "fenv_s", 0.0f }, { "aenv_s", 0.9f }, { "aenv_r", 0.1f },
            { "chorus_mix", 0.0f }, { "reverb_mix", 0.0f } } },

        { "Soft Pad", {
            { "osc1_shape", 2 }, { "uni_voices", 4 }, { "uni_spread", 20.0f },
            { "flt_cutoff", 1200.0f }, { "flt_env", 0.3f },
            { "fenv_a", 1.2f }, { "fenv_d", 2.0f }, { "fenv_s", 0.5f },
            { "aenv_a", 1.0f }, { "aenv_d", 1.0f }, { "aenv_s", 0.8f }, { "aenv_r", 2.5f },
            { "lfo_rate", 0.3f }, { "lfo_cutoff", 0.15f }, { "chorus_mix", 0.5f }, { "reverb_mix", 0.45f } } },

        { "Pluck", {
            { "uni_voices", 2 }, { "uni_spread", 8.0f }, { "flt_cutoff", 600.0f }, { "flt_env", 0.7f },
            { "fenv_d", 0.2f }, { "fenv_s", 0.0f }, { "aenv_d", 0.35f }, { "aenv_s", 0.0f }, { "aenv_r", 0.2f },
            { "reverb_mix", 0.25f },
            { "mm1_src", 5 }, { "mm1_dst", 10 }, { "mm1_amt", 0.5f } } },

        { "Arp Sequence", {
            { "arp_on", 1 }, { "arp_mode", 0 }, { "arp_rate", 2 }, { "arp_oct", 2 }, { "arp_gate", 0.5f },
            { "osc1_shape", 1 }, { "osc1_pw", 0.4f }, { "uni_voices", 2 },
            { "flt_cutoff", 1500.0f }, { "flt_env", 0.5f }, { "fenv_d", 0.2f }, { "fenv_s", 0.0f },
            { "aenv_d", 0.25f }, { "aenv_s", 0.6f }, { "aenv_r", 0.15f }, { "reverb_mix", 0.2f } } },

        { "Ring Bell", {
            { "osc1_shape", 3 }, { "osc2_shape", 3 }, { "osc2_semi", 7 }, { "osc2_fine", 0.0f },
            { "ring_level", 0.8f }, { "osc_mix", 0.3f }, { "uni_voices", 1 }, { "uni_spread", 0.0f },
            { "flt_cutoff", 8000.0f }, { "aenv_d", 1.2f }, { "aenv_s", 0.0f }, { "aenv_r", 1.5f },
            { "reverb_mix", 0.4f } } },

        { "Acid", {
            { "osc_mix", 0.0f }, { "uni_voices", 1 }, { "uni_spread", 0.0f },
            { "flt_cutoff", 500.0f }, { "flt_res", 0.75f }, { "flt_env", 0.8f },
            { "fenv_d", 0.2f }, { "fenv_s", 0.0f }, { "aenv_r", 0.1f },
            { "chorus_mix", 0.0f }, { "reverb_mix", 0.0f } } },

        { "Wobble Bass", {
            { "uni_voices", 2 }, { "uni_spread", 10.0f }, { "sub_level", 0.5f },
            { "flt_cutoff", 400.0f }, { "flt_res", 0.5f }, { "flt_env", 0.0f },
            { "lfo_rate", 2.5f }, { "lfo_shape", 1 },
            { "mm1_src", 1 }, { "mm1_dst", 6 }, { "mm1_amt", 0.5f },
            { "chorus_mix", 0.0f }, { "reverb_mix", 0.0f } } },

        { "Dual Filter Sweep", {
            { "uni_voices", 3 }, { "flt_cutoff", 3000.0f }, { "f2_type", 2 }, { "f2_cutoff", 200.0f },
            { "f2_res", 0.4f }, { "f_route", 0 }, { "lfo2_rate", 0.4f },
            { "mm1_src", 2 }, { "mm1_dst", 7 }, { "mm1_amt", 0.5f },
            { "aenv_a", 0.3f }, { "aenv_r", 1.0f }, { "chorus_mix", 0.4f }, { "reverb_mix", 0.3f } } },
    };
    return presets;
}
