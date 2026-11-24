cmake_minimum_required(VERSION 3.22)
project(FrostSynth VERSION 0.1.0)

# Neemt de Visual C++ onderdelen mee in de plugin zelf
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(JUCE)

juce_add_plugin(FrostSynth
    COMPANY_NAME "YourName"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Yrnm
    PLUGIN_CODE Frst
    FORMATS VST3 Standalone
    PRODUCT_NAME "FrostSynth")

target_sources(FrostSynth PRIVATE Source/PluginProcessor.cpp)

target_compile_features(FrostSynth PRIVATE cxx_std_17)

target_compile_definitions(FrostSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(FrostSynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
