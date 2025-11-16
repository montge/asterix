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

# Find ASTERIX core library (optional for Phase 2+)
find_library(ASTERIX_LIBRARY
    NAMES asterix libasterix
    PATHS ${CMAKE_SOURCE_DIR}/../../install/lib
          ${CMAKE_SOURCE_DIR}/../../../install/lib
          /usr/local/lib
    NO_DEFAULT_PATH
)

find_path(ASTERIX_INCLUDE_DIR
    NAMES AsterixDefinition.h
    PATHS ${CMAKE_SOURCE_DIR}/../../../install/include/asterix
          ${CMAKE_SOURCE_DIR}/../../install/include/asterix
          /usr/local/include/asterix
    NO_DEFAULT_PATH
)

# Plugin source files
set(PLUGIN_SOURCES
    packet-asterix.c
)

set(PLUGIN_HEADERS
    packet-asterix.h
)

# Add wrapper if ASTERIX core is available (Phase 2+)
if(ASTERIX_LIBRARY AND ASTERIX_INCLUDE_DIR)
    message(STATUS "ASTERIX core library found - enabling data item parsing")
    list(APPEND PLUGIN_SOURCES asterix_wrapper.cpp)
    list(APPEND PLUGIN_HEADERS asterix_wrapper.h)
    set(HAVE_ASTERIX_CORE 1)
else()
    message(STATUS "ASTERIX core library not found - Phase 1 MVP only (header parsing)")
    set(HAVE_ASTERIX_CORE 0)
endif()

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

# Add ASTERIX core includes if available
if(HAVE_ASTERIX_CORE)
    target_include_directories(${PLUGIN_NAME} PRIVATE
        ${ASTERIX_INCLUDE_DIR}
    )
    target_compile_definitions(${PLUGIN_NAME} PRIVATE
        HAVE_ASTERIX_CORE=1
    )
endif()

# Link libraries
target_link_libraries(${PLUGIN_NAME}
    ${Wireshark_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# Link ASTERIX core if available
if(HAVE_ASTERIX_CORE)
    target_link_libraries(${PLUGIN_NAME}
        ${ASTERIX_LIBRARY}
    )
endif()

# Compiler flags
target_compile_options(${PLUGIN_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
)

# C/C++ standards
set_target_properties(${PLUGIN_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# C++ standard for wrapper (if used)
if(HAVE_ASTERIX_CORE)
    set_source_files_properties(asterix_wrapper.cpp PROPERTIES
        COMPILE_FLAGS "-std=c++17"
    )
endif()

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
