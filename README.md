# FrostSynth

Polyfone virtueel-analoge synth (JUCE, VST3 + Standalone).

Features: 3 oscillatoren (saw/pulse/tri/sine, PolyBLEP) met unison, sub, noise, ringmod,
2 filters (LP/HP/BP, serieel of parallel), 2 ADSR, 2 LFO's, modulatiematrix (4 slots),
arpeggiator (host-tempo of handmatig), chorus, reverb en 10 factory presets
(plus Save/Load van eigen presets als .frostpreset).

## Bouwen
1. `git clone https://github.com/juce-framework/JUCE.git` (in deze map)
2. `cmake -B build`
3. `cmake --build build --config Release`

Het .vst3-bestand staat in `build/FrostSynth_artefacts/Release/VST3/`.

## Windows + FL Studio
Optie A (op je laptop): installeer Visual Studio Community 2022 (workload "Desktop development with C++"),
Git en CMake, en dubbelklik daarna `build_windows.bat` (bij een foutmelding over kopiëren: als administrator uitvoeren).

Optie B (zonder installeren): zet de map op GitHub, open het tabblad Actions, wacht tot de build klaar is
en download "FrostSynth-VST3-Windows". Zet de map `FrostSynth.vst3` in `C:\Program Files\Common Files\VST3`.

Daarna in FL Studio: Options > Manage plugins > Find installed plugins.
