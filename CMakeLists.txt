cmake_minimum_required(VERSION 3.23.1)

# --- General Project Setup ---
set(PROJECT_NAME "Garrotxeta")
set(CURRENT_VERSION "0.0.1")
set(PRODUCT_NAME "Garrotxeta")
set(COMPANY_NAME "AlbertPhaseLab")
set(BUNDLE_ID "com.albertphaselab.Garrotxeta")
set(FORMATS Standalone VST3 AU) 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Garrotxeta VERSION ${CURRENT_VERSION})

# --- JUCE Framework Setup ---
add_subdirectory(modules/JUCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "" FORCE) 
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)
endif()

# --- Plugin Target Definition ---
juce_add_plugin("${PROJECT_NAME}"
    COMPANY_NAME "${COMPANY_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "APlb"
    PLUGIN_CODE "GTXA"
    FORMATS ${FORMATS}
    PRODUCT_NAME "${PRODUCT_NAME}"
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE
)

# --- Source Files Section ---
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h"
)
target_sources("${PROJECT_NAME}" PRIVATE ${SourceFiles})

# --- Assets & Binary Data Section (Synced with your assets folder) ---
set(ASSET_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/background.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/garrotxeta.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/knob.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/garrotxeta_catala.jpg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/garrotxeta_barbar.jpg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/albertphaselab_logo.jpg"
)

# Validation: Warn if an asset file is missing
foreach(FILE_PATH ${ASSET_FILES})
    if(NOT EXISTS ${FILE_PATH})
        message(WARNING "MISSING ASSET: File not found at ${FILE_PATH}")
    endif()
endforeach()

# Generate C++ BinaryData from assets
juce_add_binary_data(AudioPluginData SOURCES ${ASSET_FILES})

# Link binary data to the plugin target
target_link_libraries("${PROJECT_NAME}" PRIVATE AudioPluginData)

# --- Compiler Configuration ---
target_compile_definitions("${PROJECT_NAME}" PRIVATE
    JUCE_WEB_BROWSER=0 
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    VERSION="${CURRENT_VERSION}"
)

target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source")

# --- JUCE Module Linking ---
target_link_libraries("${PROJECT_NAME}" PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_graphics
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)