#pragma once
#include "PluginProcessor.h"

class FrostSynthEditor : public juce::AudioProcessorEditor
{
public:
    explicit FrostSynthEditor (FrostSynthProcessor& p) : AudioProcessorEditor (p), proc (p)
    {
        // Linker paneel
        addRow (0, "OSC 1",    { "osc1_shape", "osc1_semi", "osc1_fine", "osc1_pw" });
        addRow (0, "OSC 2",    { "osc2_shape", "osc2_semi", "osc2_fine", "osc2_pw" });
        addRow (0, "OSC 3",    { "osc3_shape", "osc3_semi", "osc3_fine", "osc3_pw", "osc3_level", "ring_level" });
        addRow (0, "MIX",      { "osc_mix", "sub_level", "sub_shape", "noise_level", "uni_voices", "uni_spread" });
        addRow (0, "FILTER 1", { "flt_type", "flt_slope", "flt_cutoff", "flt_res", "flt_env", "flt_key" });
        addRow (0, "FILTER 2", { "f2_type", "f_route", "f2_cutoff", "f2_res", "f2_env", "f2_key" });

        // Rechter paneel
        addRow (1, "ENVELOPES", { "fenv_a", "fenv_d", "fenv_s", "fenv_r", "aenv_a", "aenv_d", "aenv_s", "aenv_r" });
        addRow (1, "LFO",       { "lfo_rate", "lfo_shape", "lfo_pitch", "lfo_cutoff", "lfo2_rate", "lfo2_shape" });
        addRow (1, "MOD 1-2",   { "mm1_src", "mm1_dst", "mm1_amt", "mm2_src", "mm2_dst", "mm2_amt" });
        addRow (1, "MOD 3-4",   { "mm3_src", "mm3_dst", "mm3_amt", "mm4_src", "mm4_dst", "mm4_amt" });
        addRow (1, "ARP",       { "arp_on", "arp_mode", "arp_rate", "arp_oct", "arp_gate", "arp_sync", "arp_bpm" });
        addRow (1, "FX",        { "chorus_mix", "reverb_mix", "master" });

        for (int i = 0; i < proc.getNumPrograms(); ++i)
            presetBox.addItem (proc.getProgramName (i), i + 1);
        presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
        presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
        addAndMakeVisible (presetBox);

        addAndMakeVisible (saveBtn);
        addAndMakeVisible (loadBtn);
        saveBtn.onClick = [this] { savePreset(); };
        loadBtn.onClick = [this] { loadPreset(); };

        setSize (2 * panelW, headerH + 6 * rowH + 15);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff10141c));

        g.setColour (juce::Colour (0xff7fd4ff));
        g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
        g.drawText ("FrostSynth", getWidth() - 200, 6, 190, 30, juce::Justification::centredRight);

        g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
        for (auto& r : rows)
            g.drawText (r.title, r.panel * panelW + 10, headerH + r.index * rowH, titleW - 10, rowH,
                        juce::Justification::centredLeft);

        g.setColour (juce::Colour (0xff2a3140));
        g.drawVerticalLine (panelW, (float) headerH, (float) getHeight());
    }

    void resized() override
    {
        presetBox.setBounds (10, 8, 240, 26);
        saveBtn.setBounds (260, 8, 60, 26);
        loadBtn.setBounds (325, 8, 60, 26);

        for (auto& r : rows)
        {
            for (size_t c = 0; c < r.cells.size(); ++c)
            {
                auto& cell = *r.cells[c];
                const int x = r.panel * panelW + titleW + (int) c * cellW;
                const int y = headerH + r.index * rowH;
                cell.label.setBounds (x, y, cellW, 16);

                if (cell.kind == Cell::Choice)
                    cell.combo.setBounds (x + 3, y + 34, cellW - 6, 24);
                else if (cell.kind == Cell::Toggle)
                    cell.toggle.setBounds (x + 12, y + 34, cellW - 24, 24);
                else
                    cell.slider.setBounds (x, y + 16, cellW, rowH - 20);
            }
        }
    }

private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct Cell
    {
        enum Kind { Knob, Choice, Toggle };
        Kind kind = Knob;
        juce::Label label;
        juce::Slider slider;
        juce::ComboBox combo;
        juce::ToggleButton toggle { "On" };
        std::unique_ptr<SA> sa;
        std::unique_ptr<CA> ca;
        std::unique_ptr<BA> ba;
    };

    struct Row
    {
        int panel = 0, index = 0;
        juce::String title;
        std::vector<std::unique_ptr<Cell>> cells;
    };

    void addRow (int panel, const juce::String& title, juce::StringArray ids)
    {
        Row row;
        row.panel = panel;
        row.index = rowCount[panel]++;
        row.title = title;
        for (auto& id : ids)
            row.cells.push_back (makeCell (id));
        rows.push_back (std::move (row));
    }

    std::unique_ptr<Cell> makeCell (const juce::String& id)
    {
        auto c = std::make_unique<Cell>();
        auto* param = proc.apvts.getParameter (id);

        c->label.setText (param->getName (14), juce::dontSendNotification);
        c->label.setJustificationType (juce::Justification::centred);
        c->label.setFont (juce::FontOptions (12.0f));
        addAndMakeVisible (c->label);

        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param))
        {
            c->kind = Cell::Choice;
            c->combo.addItemList (choice->choices, 1);
            addAndMakeVisible (c->combo);
            c->ca = std::make_unique<CA> (proc.apvts, id, c->combo);
        }
        else if (dynamic_cast<juce::AudioParameterBool*> (param) != nullptr)
        {
            c->kind = Cell::Toggle;
            addAndMakeVisible (c->toggle);
            c->ba = std::make_unique<BA> (proc.apvts, id, c->toggle);
        }
        else
        {
            c->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            c->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 66, 16);
            addAndMakeVisible (c->slider);
            c->sa = std::make_unique<SA> (proc.apvts, id, c->slider);
        }
        return c;
    }

    void savePreset()
    {
        chooser = std::make_unique<juce::FileChooser> ("Save preset",
            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.frostpreset");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode
                              | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
            [this] (const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File())
                    return;
                if (auto xml = proc.apvts.copyState().createXml())
                    xml->writeTo (f.withFileExtension ("frostpreset"));
            });
    }

    void loadPreset()
    {
        chooser = std::make_unique<juce::FileChooser> ("Load preset",
            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.frostpreset");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (! f.existsAsFile())
                    return;
                if (auto xml = juce::XmlDocument::parse (f))
                    if (xml->hasTagName (proc.apvts.state.getType()))
                        proc.apvts.replaceState (juce::ValueTree::fromXml (*xml));
            });
    }

    static constexpr int titleW = 90, cellW = 80, rowH = 100, headerH = 45;
    static constexpr int panelW = titleW + 8 * cellW;

    FrostSynthProcessor& proc;
    std::vector<Row> rows;
    int rowCount[2] { 0, 0 };
    juce::ComboBox presetBox;
    juce::TextButton saveBtn { "Save" }, loadBtn { "Load" };
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrostSynthEditor)
};
