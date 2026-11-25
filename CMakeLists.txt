cmake_minimum_required(VERSION 3.22)
project(R2RVERB VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(R2RVERB
    COMPANY_NAME "DJ R2R Music"
    BUNDLE_ID "com.djr2rmusic.r2rverb"
    PLUGIN_MANUFACTURER_CODE DjR2
    PLUGIN_CODE R2Rv
    PRODUCT_NAME "R2RVERB"
    DESCRIPTION "DJ R2R Music Advanced Hall / Room / Plate Reverb"
    FORMATS VST3
    VST3_CATEGORIES Fx Reverb
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Generate JuceHeader.h required by the plugin source files.
juce_generate_juce_header(R2RVERB)

target_sources(R2RVERB PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(R2RVERB PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(R2RVERB PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
