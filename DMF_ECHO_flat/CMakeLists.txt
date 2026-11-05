cmake_minimum_required(VERSION 3.22)
project(DMF_ECHO VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare( JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE )
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(DMF_ECHO
    COMPANY_NAME "DIGITAL MAFIA"
    BUNDLE_ID "com.digitalmafia.dmfecho"
    PLUGIN_MANUFACTURER_CODE DgMf
    PLUGIN_CODE DmEc
    FORMATS VST3 Standalone
    PRODUCT_NAME "DMF ECHO"
    DESCRIPTION "Musical Delay"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)
juce_generate_juce_header(DMF_ECHO)

target_sources(DMF_ECHO PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    DSP.h
)

target_compile_definitions(DMF_ECHO PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(DMF_ECHO PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags
)
