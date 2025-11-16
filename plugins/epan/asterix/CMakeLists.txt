# CMakeLists.txt for Wireshark ASTERIX Plugin
#
# This builds the ASTERIX dissector plugin for Wireshark 4.0+
#
# Build instructions:
#   1. Install Wireshark development packages:
#      Ubuntu/Debian: sudo apt-get install wireshark-dev libwireshark-dev
#      Fedora: sudo dnf install wireshark-devel
#      macOS: brew install wireshark
#
#   2. Configure and build:
#      cmake -B build -DBUILD_WIRESHARK_PLUGIN=ON
#      cmake --build build
#
#   3. Install plugin:
#      mkdir -p ~/.local/lib/wireshark/plugins/4.0/epan
#      cp build/plugins/epan/asterix/asterix.so ~/.local/lib/wireshark/plugins/4.0/epan/
#
# Requirements:
#   - Wireshark 4.0.0 or later
#   - CMake 3.12+
#   - GLib 2.50+

cmake_minimum_required(VERSION 3.12)

# Plugin metadata
set(PLUGIN_NAME asterix)
set(PLUGIN_VERSION_MAJOR 1)
set(PLUGIN_VERSION_MINOR 0)
set(PLUGIN_VERSION_PATCH 0)
set(PLUGIN_VERSION "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}.${PLUGIN_VERSION_PATCH}")

project(wireshark-${PLUGIN_NAME}-plugin VERSION ${PLUGIN_VERSION})

# Find Wireshark
find_package(Wireshark 4.0 REQUIRED)

if(NOT Wireshark_FOUND)
    message(FATAL_ERROR "Wireshark 4.0+ not found. Install Wireshark development packages.")
endif()

message(STATUS "Found Wireshark ${Wireshark_VERSION}")
message(STATUS "Wireshark include dir: ${Wireshark_INCLUDE_DIRS}")
message(STATUS "Wireshark plugin dir: ${Wireshark_PLUGIN_INSTALL_DIR}")

# Find GLib (required by Wireshark)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.50)

if(NOT GLIB2_FOUND)
    message(FATAL_ERROR "GLib 2.50+ not found")
endif()

# Plugin source files
set(PLUGIN_SOURCES
    packet-asterix.c
)

set(PLUGIN_HEADERS
    packet-asterix.h
)

# Generate moduleinfo.h from template
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/moduleinfo.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/moduleinfo.h
    @ONLY
)

# Create plugin library
add_library(${PLUGIN_NAME} MODULE
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

# Include directories
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Wireshark_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PLUGIN_NAME}
    ${Wireshark_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# Compiler flags
target_compile_options(${PLUGIN_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
)

# C standard
set_target_properties(${PLUGIN_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# Remove 'lib' prefix from output filename (plugin should be asterix.so, not libasterix.so)
set_target_properties(${PLUGIN_NAME} PROPERTIES
    PREFIX ""
)

# Installation
if(Wireshark_PLUGIN_INSTALL_DIR)
    install(TARGETS ${PLUGIN_NAME}
        LIBRARY DESTINATION ${Wireshark_PLUGIN_INSTALL_DIR}/epan
    )
    message(STATUS "Plugin will be installed to: ${Wireshark_PLUGIN_INSTALL_DIR}/epan")
else()
    # Fallback for manual installation
    message(STATUS "Wireshark plugin directory not found.")
    message(STATUS "After building, manually copy asterix.so to:")
    message(STATUS "  Linux:   ~/.local/lib/wireshark/plugins/4.0/epan/")
    message(STATUS "  macOS:   ~/Library/Application Support/Wireshark/plugins/4.0/epan/")
    message(STATUS "  Windows: %APPDATA%\\Wireshark\\plugins\\4.0\\epan\\")
endif()

# Testing support
enable_testing()

# Display build information
message(STATUS "")
message(STATUS "=== Wireshark ASTERIX Plugin Configuration ===")
message(STATUS "  Plugin name:    ${PLUGIN_NAME}")
message(STATUS "  Plugin version: ${PLUGIN_VERSION}")
message(STATUS "  Wireshark ver:  ${Wireshark_VERSION}")
message(STATUS "  GLib version:   ${GLIB2_VERSION}")
message(STATUS "  C standard:     C11")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "===============================================")
message(STATUS "")
