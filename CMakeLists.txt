cmake_minimum_required(VERSION 3.24)

project(SpectraReverb VERSION 1.0.0 LANGUAGES CXX)

option(SPECTRA_BUILD_AAX "Build the AAX plugin (requires the Avid AAX SDK)" OFF)

include(FetchContent)

set(SPECTRA_PLUGIN_FORMATS VST3)
if(APPLE)
    list(APPEND SPECTRA_PLUGIN_FORMATS AU)
endif()

if(SPECTRA_BUILD_AAX)
    if(NOT JUCE_AAX_SDK_PATH OR NOT EXISTS "${JUCE_AAX_SDK_PATH}")
        message(FATAL_ERROR "AAX requires a valid JUCE_AAX_SDK_PATH. Obtain the Avid AAX SDK, then configure with -DSPECTRA_BUILD_AAX=ON -DJUCE_AAX_SDK_PATH=<SDK path>.")
    endif()
    list(APPEND SPECTRA_PLUGIN_FORMATS AAX)
endif()

FetchContent_Declare(
    JUCE
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.6.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SpectraReverb
    COMPANY_NAME "Auralis Audio"
    BUNDLE_ID "audio.auralis.spectrareverb"
    PLUGIN_MANUFACTURER_CODE Aurl
    PLUGIN_CODE SpRv
    PRODUCT_NAME "Spectra Reverb"
    DESCRIPTION "Stereo algorithmic reverb"
    FORMATS ${SPECTRA_PLUGIN_FORMATS}
    VST3_CATEGORIES Fx Reverb
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(SpectraReverb)

target_sources(SpectraReverb
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/ReverbEngine.cpp
)

target_compile_features(SpectraReverb PRIVATE cxx_std_17)

target_compile_definitions(SpectraReverb
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SpectraReverb
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
