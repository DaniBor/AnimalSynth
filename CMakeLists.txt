cmake_minimum_required(VERSION 3.15)

project(AnimalSynth VERSION 1.0.0)

# Add JUCE as a subdirectory (make sure it's correctly added as submodule or source)
add_subdirectory(JUCE)

# === Gather source files ===
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    Source/*.cpp
    Source/*.h
)

# === Collect image assets for embedding ===
file(GLOB_RECURSE ASSET_IMAGES CONFIGURE_DEPENDS
    assets/*.png
    assets/*.jpg
    assets/*.jpeg
)

# === Add binary data (creates BinaryData.{cpp,h} automatically) ===
juce_add_binary_data(BinaryData
    SOURCES ${ASSET_IMAGES}
)

# === Define the plugin target ===
juce_add_plugin(AnimalSynth
    COMPANY_NAME                "Your Name"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    PLUGIN_MANUFACTURER_CODE    Juce
    PLUGIN_CODE                 Asyn
    FORMATS                     VST3 AU Standalone
    PRODUCT_NAME                "AnimalSynth"
    VST3_REPLACES_VST2          FALSE
)

# === Add sources to plugin target ===
target_sources(AnimalSynth PRIVATE ${SOURCE_FILES})

# === Include BinaryData in include path (for BinaryData.h) ===
target_include_directories(AnimalSynth PRIVATE
    ${CMAKE_BINARY_DIR}/BinaryData
)

# === Link BinaryData and JUCE modules ===
target_link_libraries(AnimalSynth PRIVATE
    BinaryData

    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# === Optional: Suppress VST3 parameter ID warnings ===
target_compile_definitions(AnimalSynth PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
)
