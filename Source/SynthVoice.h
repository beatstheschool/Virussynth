#pragma once
#include "Params.h"

struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice (juce::AudioProcessorValueTreeState& apvts, SharedState& s) : shared (s)
    {
        for (int i = 0; i < P::count; ++i)
            ptrs[i] = apvts.getRawParameterValue (P::name (i));
    }

    void prepare (double sr, int block)
    {
        sampleRate = sr;
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, 1 };
        for (auto& f : filt) { f.prepare (spec); f.reset(); }
        filt2.prepare (spec);
        filt2.reset();
        ampEnv.setSampleRate (sr);
        fltEnv.setSampleRate (sr);
    }

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<SynthSound*> (s) != nullptr; }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheel) override
    {
        note = midiNote;
        vel = velocity;
        bend = (pitchWheel - 8192) / 8192.0f * 2.0f;
        for (auto& p : ph1) p = rnd.nextFloat();
        for (auto& p : ph2) p = rnd.nextFloat();
        for (auto& p : ph3) p = rnd.nextFloat();
        subPhase = 0.0f;
        lfoPhase = 0.0f;
        lfo2Phase = 0.0f;
        noteRand = rnd.nextFloat() * 2.0f - 1.0f;
        lastAmp = 0.0f;
        counter = 0;
        for (auto& f : filt) f.reset();
        filt2.reset();
        updateEnvParams();
        ampEnv.noteOn();
        fltEnv.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            ampEnv.noteOff();
            fltEnv.noteOff();
        }
        else
        {
            ampEnv.reset();
            fltEnv.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved (int val) override { bend = (val - 8192) / 8192.0f * 2.0f; }
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (getCurrentlyPlayingNote() < 0)
            return;

        updateEnvParams();

        nUni = juce::jlimit (1, 5, (int) v (P::uni_voices));
        const float norm    = 1.0f / std::sqrt ((float) nUni);
        const int   shp[3]  = { (int) v (P::osc1_shape), (int) v (P::osc2_shape), (int) v (P::osc3_shape) };
        const float lvl3    = v (P::osc3_level);
        const float ring    = v (P::ring_level);
        const float sub     = v (P::sub_level);
        const int   subShp  = (int) v (P::sub_shape);
        const float noise   = v (P::noise_level);
        const bool  slope24 = v (P::flt_slope) > 0.5f;
        const bool  parallel = v (P::f_route) > 0.5f;
        const int   type1   = (int) v (P::flt_type);   // LP, HP, BP
        const int   type2   = (int) v (P::f2_type);    // Off, LP, HP, BP
        const bool  f2on    = type2 > 0;

        auto toType = [] (int t)
        {
            return t == 0 ? juce::dsp::StateVariableTPTFilterType::lowpass
                 : t == 1 ? juce::dsp::StateVariableTPTFilterType::highpass
                          : juce::dsp::StateVariableTPTFilterType::bandpass;
        };
        for (auto& f : filt) f.setType (toType (type1));
        if (f2on) filt2.setType (toType (type2 - 1));

        for (int i = 0; i < num; ++i)
        {
            const float a  = ampEnv.getNextSample();
            const float fe = fltEnv.getNextSample();
            lastAmp = a;
            if ((counter++ & 15) == 0)
                control (fe);

            float o1 = 0.0f, o2 = 0.0f, o3 = 0.0f;
            for (int u = 0; u < nUni; ++u)
            {
                o1 += oscSample (shp[0], ph1[u], dt1[u], pwM[0]);
                o2 += oscSample (shp[1], ph2[u], dt2[u], pwM[1]);
                o3 += oscSample (shp[2], ph3[u], dt3[u], pwM[2]);
                ph1[u] += dt1[u]; if (ph1[u] >= 1.0f) ph1[u] -= 1.0f;
                ph2[u] += dt2[u]; if (ph2[u] >= 1.0f) ph2[u] -= 1.0f;
                ph3[u] += dt3[u]; if (ph3[u] >= 1.0f) ph3[u] -= 1.0f;
            }
            o1 *= norm; o2 *= norm; o3 *= norm;

            float x = (1.0f - mixM) * o1 + mixM * o2 + lvl3 * o3 + ring * o1 * o2;

            const float s = subShp == 0 ? oscSample (1, subPhase, dtSub, 0.5f)
                                        : std::sin (juce::MathConstants<float>::twoPi * subPhase);
            subPhase += dtSub; if (subPhase >= 1.0f) subPhase -= 1.0f;
            x += sub * s;
            x += noise * (rnd.nextFloat() * 2.0f - 1.0f);

            float y = filt[0].processSample (0, x);
            if (slope24)
                y = filt[1].processSample (0, y);

            if (f2on)
            {
                if (parallel) y = 0.5f * (y + filt2.processSample (0, x));
                else          y = filt2.processSample (0, y);
            }

            y *= a * vel * 0.35f * ampGain;

            for (int ch = 0; ch < out.getNumChannels(); ++ch)
                out.addSample (ch, start + i, y * (ch == 0 ? gL : gR));

            if (! ampEnv.isActive())
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    float v (P::Id id) const { return ptrs[id]->load(); }

    void updateEnvParams()
    {
        ampEnv.setParameters ({ v (P::aenv_a), v (P::aenv_d), v (P::aenv_s), v (P::aenv_r) });
        fltEnv.setParameters ({ v (P::fenv_a), v (P::fenv_d), v (P::fenv_s), v (P::fenv_r) });
    }

    static float wrap01 (float p) { return p - std::floor (p); }

    static float lfoWave (int shape, float ph)
    {
        switch (shape)
        {
            case 0:  return std::sin (juce::MathConstants<float>::twoPi * ph);
            case 1:  return 4.0f * std::abs (ph - 0.5f) - 1.0f;
            case 2:  return ph < 0.5f ? 1.0f : -1.0f;
            default: return 2.0f * ph - 1.0f;
        }
    }

    // Control rate: elke 16 samples. Berekent LFO's, modulatiematrix, pitch en filters.
    void control (float fe)
    {
        const float sr = (float) sampleRate;

        lfoPhase  = wrap01 (lfoPhase  + v (P::lfo_rate)  * 16.0f / sr);
        lfo2Phase = wrap01 (lfo2Phase + v (P::lfo2_rate) * 16.0f / sr);
        const float l1 = lfoWave ((int) v (P::lfo_shape),  lfoPhase);
        const float l2 = lfoWave ((int) v (P::lfo2_shape), lfo2Phase);

        const float src[10] = { 0.0f, l1, l2, fe, lastAmp, vel,
                                juce::jlimit (-1.0f, 1.0f, ((float) note - 60.0f) / 60.0f),
                                shared.modWheel.load(), shared.aftertouch.load(), noteRand };

        float dst[12] {};
        for (int k = 0; k < 4; ++k)
        {
            const int s = (int) v ((P::Id) (P::mm1_src + 3 * k));
            const int d = (int) v ((P::Id) (P::mm1_dst + 3 * k));
            if (s > 0 && d > 0)
                dst[d] += src[s] * v ((P::Id) (P::mm1_amt + 3 * k));
        }

        // Pitch
        const float semi = bend + l1 * v (P::lfo_pitch) + dst[1] * 12.0f;
        auto hz = [&] (float s) { return 440.0f * std::pow (2.0f, ((float) note + s - 69.0f) / 12.0f); };

        const float f1 = hz (semi + v (P::osc1_semi) + v (P::osc1_fine) / 100.0f + dst[2] * 12.0f);
        const float f2 = hz (semi + v (P::osc2_semi) + v (P::osc2_fine) / 100.0f + dst[3] * 12.0f);
        const float f3 = hz (semi + v (P::osc3_semi) + v (P::osc3_fine) / 100.0f + dst[4] * 12.0f);
        const float spread = v (P::uni_spread);

        for (int u = 0; u < nUni; ++u)
        {
            const float pos   = nUni > 1 ? ((float) u / (float) (nUni - 1)) * 2.0f - 1.0f : 0.0f;
            const float ratio = std::pow (2.0f, pos * spread / 1200.0f);
            dt1[u] = juce::jmin (0.45f, f1 * ratio / sr);
            dt2[u] = juce::jmin (0.45f, f2 * ratio / sr);
            dt3[u] = juce::jmin (0.45f, f3 * ratio / sr);
        }
        dtSub = juce::jmin (0.45f, hz (semi + v (P::osc1_semi) - 12.0f) / sr);

        // Pulsbreedte, mix, level, pan
        pwM[0] = juce::jlimit (0.05f, 0.95f, v (P::osc1_pw) + dst[5] * 0.45f);
        pwM[1] = juce::jlimit (0.05f, 0.95f, v (P::osc2_pw) + dst[5] * 0.45f);
        pwM[2] = juce::jlimit (0.05f, 0.95f, v (P::osc3_pw) + dst[5] * 0.45f);
        mixM    = juce::jlimit (0.0f, 1.0f, v (P::osc_mix) + dst[9] * 0.5f);
        ampGain = juce::jlimit (0.0f, 2.0f, 1.0f + dst[10]);
        const float pan   = juce::jlimit (-1.0f, 1.0f, dst[11]);
        const float angle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        gL = std::cos (angle) * 1.4142f;
        gR = std::sin (angle) * 1.4142f;

        // Filters
        const float maxHz = (float) juce::jmin (20000.0, sampleRate * 0.45);
        const float key   = ((float) note - 60.0f) / 12.0f;
        const float res   = juce::jlimit (0.0f, 1.0f, v (P::flt_res) + dst[8] * 0.5f);

        const float oct1 = v (P::flt_env) * fe * 5.0f + l1 * v (P::lfo_cutoff) * 3.0f
                         + v (P::flt_key) * key + dst[6] * 5.0f;
        const float c1 = juce::jlimit (20.0f, maxHz, v (P::flt_cutoff) * std::pow (2.0f, oct1));
        const float q1 = 0.7f + res * res * 12.0f;
        for (auto& f : filt) { f.setCutoffFrequency (c1); f.setResonance (q1); }

        const float oct2 = v (P::f2_env) * fe * 5.0f + v (P::f2_key) * key + dst[7] * 5.0f;
        const float c2 = juce::jlimit (20.0f, maxHz, v (P::f2_cutoff) * std::pow (2.0f, oct2));
        const float r2 = v (P::f2_res);
        filt2.setCutoffFrequency (c2);
        filt2.setResonance (0.7f + r2 * r2 * 12.0f);
    }

    static float polyBlep (float t, float dt)
    {
        if (t < dt)        { t /= dt;             return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    static float oscSample (int shape, float ph, float dt, float pw)
    {
        switch (shape)
        {
            case 0: return 2.0f * ph - 1.0f - polyBlep (ph, dt);              // Saw
            case 1:                                                             // Pulse
            {
                float val = ph < pw ? 1.0f : -1.0f;
                val += polyBlep (ph, dt);
                float t = ph - pw; if (t < 0.0f) t += 1.0f;
                val -= polyBlep (t, dt);
                return val;
            }
            case 2:  return 4.0f * std::abs (ph - 0.5f) - 1.0f;                // Triangle
            default: return std::sin (juce::MathConstants<float>::twoPi * ph); // Sine
        }
    }

    SharedState& shared;
    std::atomic<float>* ptrs[P::count] {};
    juce::dsp::StateVariableTPTFilter<float> filt[2], filt2;
    juce::ADSR ampEnv, fltEnv;
    juce::Random rnd;

    double sampleRate = 44100.0;
    int note = 60, counter = 0, nUni = 1;
    float vel = 1.0f, bend = 0.0f, noteRand = 0.0f, lastAmp = 0.0f;
    float lfoPhase = 0.0f, lfo2Phase = 0.0f, subPhase = 0.0f, dtSub = 0.0f;
    float mixM = 0.5f, ampGain = 1.0f, gL = 1.0f, gR = 1.0f;
    float pwM[3] { 0.5f, 0.5f, 0.5f };
    float ph1[5] {}, ph2[5] {}, ph3[5] {}, dt1[5] {}, dt2[5] {}, dt3[5] {};
};
