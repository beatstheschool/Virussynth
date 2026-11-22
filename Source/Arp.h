#pragma once
#include "Params.h"
#include <algorithm>

// Arpeggiator: leest noten uit de MIDI-buffer en schrijft er arp-noten voor terug.
class Arpeggiator
{
public:
    struct Settings
    {
        bool on = false;
        int mode = 0;       // 0 Up, 1 Down, 2 UpDown, 3 Random, 4 As Played
        int rateIdx = 2;    // 0 = 1/4, 1 = 1/8, 2 = 1/16, 3 = 1/32
        int octaves = 1;
        float gate = 0.6f;
        double bpm = 120.0;
    };

    void reset()
    {
        held.clear();
        sounding = -1;
        nextStep = 0.0;
        gateLeft = 0.0;
        stepIdx = 0;
        active = false;
    }

    void process (juce::MidiBuffer& midi, int numSamples, double sr, const Settings& s)
    {
        if (! s.on)
        {
            if (active)
            {
                juce::MidiBuffer out;
                if (sounding >= 0)
                    out.addEvent (juce::MidiMessage::noteOff (1, sounding), 0);
                for (auto md : midi)
                    out.addEvent (md.getMessage(), md.samplePosition);
                midi.swapWith (out);
                reset();
            }
            return;
        }

        active = true;
        static const double beats[] = { 1.0, 0.5, 0.25, 0.125 };
        const double stepLen = sr * 60.0 / juce::jmax (20.0, s.bpm) * beats[juce::jlimit (0, 3, s.rateIdx)];

        juce::MidiBuffer out;
        auto it = midi.begin();

        for (int pos = 0; pos < numSamples; ++pos)
        {
            while (it != midi.end() && (*it).samplePosition <= pos)
            {
                const auto m = (*it).getMessage();
                ++it;

                if (m.isNoteOn())
                {
                    const int n = m.getNoteNumber();
                    if (std::find (held.begin(), held.end(), n) == held.end())
                        held.push_back (n);
                    lastVel = m.getFloatVelocity();
                    if (held.size() == 1)
                    {
                        nextStep = 0.0;
                        stepIdx = 0;
                    }
                }
                else if (m.isNoteOff())
                {
                    held.erase (std::remove (held.begin(), held.end(), m.getNoteNumber()), held.end());
                    if (held.empty())
                    {
                        if (sounding >= 0)
                            out.addEvent (juce::MidiMessage::noteOff (1, sounding), pos);
                        sounding = -1;
                        nextStep = 0.0;
                    }
                }
                else if (m.isAllNotesOff() || m.isAllSoundOff())
                {
                    held.clear();
                    if (sounding >= 0)
                        out.addEvent (juce::MidiMessage::noteOff (1, sounding), pos);
                    sounding = -1;
                }
                else
                {
                    out.addEvent (m, pos);   // pitch bend, CC, enz. gaan gewoon door
                }
            }

            if (sounding >= 0)
            {
                gateLeft -= 1.0;
                if (gateLeft <= 0.0)
                {
                    out.addEvent (juce::MidiMessage::noteOff (1, sounding), pos);
                    sounding = -1;
                }
            }

            if (! held.empty())
            {
                nextStep -= 1.0;
                if (nextStep <= 0.0)
                {
                    const auto seq = buildSequence (s);
                    const int note = s.mode == 3 ? seq[(size_t) rnd.nextInt ((int) seq.size())]
                                                 : seq[(size_t) (stepIdx++ % (int) seq.size())];

                    if (sounding >= 0)
                        out.addEvent (juce::MidiMessage::noteOff (1, sounding), pos);
                    out.addEvent (juce::MidiMessage::noteOn (1, note, lastVel), pos);
                    sounding = note;
                    gateLeft = stepLen * (double) juce::jlimit (0.05f, 1.0f, s.gate);
                    nextStep += stepLen;
                }
            }
            else
            {
                nextStep = 0.0;
            }
        }

        midi.swapWith (out);
    }

private:
    std::vector<int> buildSequence (const Settings& s) const
    {
        std::vector<int> base = held;
        if (s.mode != 4)
            std::sort (base.begin(), base.end());

        std::vector<int> up;
        for (int o = 0; o < juce::jlimit (1, 4, s.octaves); ++o)
            for (int n : base)
                if (n + 12 * o <= 127)
                    up.push_back (n + 12 * o);
        if (up.empty())
            up = base;

        if (s.mode == 1)
        {
            std::reverse (up.begin(), up.end());
        }
        else if (s.mode == 2)
        {
            auto seq = up;
            for (int i = (int) up.size() - 2; i > 0; --i)
                seq.push_back (up[(size_t) i]);
            return seq;
        }
        return up;
    }

    std::vector<int> held;
    juce::Random rnd;
    int sounding = -1, stepIdx = 0;
    double nextStep = 0.0, gateLeft = 0.0;
    float lastVel = 0.8f;
    bool active = false;
};
