# CMakeLists.txt for ASTERIX Wireshark 4.x Plugin
#
# This builds the ASTERIX protocol dissector plugin for Wireshark 4.x
#
# Requirements:
#   - Wireshark 4.0+ development headers
#   - ASTERIX library (libasterix)
#   - CMake 3.20+
#
# Usage:
#   mkdir build && cd build
#   cmake .. -DWIRESHARK_INCLUDE_DIR=/path/to/wireshark/include
#   cmake --build .
#   cmake --install . --prefix ~/.local/lib/wireshark/plugins/4.x/epan/

cmake_minimum_required(VERSION 3.20)
project(asterix-wireshark-plugin
    VERSION 2.9.0
    LANGUAGES C CXX
    DESCRIPTION "ASTERIX Protocol Dissector for Wireshark 4.x"
)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Find Wireshark
# Try pkg-config first
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(WIRESHARK wireshark>=4.0)
endif()

# Manual Wireshark configuration if pkg-config fails
if(NOT WIRESHARK_FOUND)
    # Allow user to specify Wireshark paths
    set(WIRESHARK_INCLUDE_DIR "" CACHE PATH "Path to Wireshark include directory")
    set(WIRESHARK_LIBRARY_DIR "" CACHE PATH "Path to Wireshark library directory")

    if(WIRESHARK_INCLUDE_DIR)
        set(WIRESHARK_INCLUDE_DIRS ${WIRESHARK_INCLUDE_DIR})
        set(WIRESHARK_FOUND TRUE)
    else()
        # Try common installation paths
        find_path(WIRESHARK_INCLUDE_DIRS
            NAMES epan/packet.h
            PATHS
                /usr/include/wireshark
                /usr/local/include/wireshark
                /opt/wireshark/include
                $ENV{HOME}/wireshark/include
            PATH_SUFFIXES wireshark
        )

        if(WIRESHARK_INCLUDE_DIRS)
            set(WIRESHARK_FOUND TRUE)
        endif()
    endif()

    if(WIRESHARK_LIBRARY_DIR)
        set(WIRESHARK_LIBRARY_DIRS ${WIRESHARK_LIBRARY_DIR})
    else()
        set(WIRESHARK_LIBRARY_DIRS
            /usr/lib
            /usr/lib/x86_64-linux-gnu
            /usr/local/lib
            /opt/wireshark/lib
        )
    endif()
endif()

if(NOT WIRESHARK_FOUND)
    message(WARNING "Wireshark development headers not found. Plugin will not be built.")
    message(STATUS "Set WIRESHARK_INCLUDE_DIR to the Wireshark include path")
    message(STATUS "Or install wireshark-dev package: apt install wireshark-dev")
    return()
endif()

message(STATUS "Wireshark include dirs: ${WIRESHARK_INCLUDE_DIRS}")

# Find GLib (required by Wireshark)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.50)

# Find ASTERIX library
find_library(ASTERIX_LIBRARY
    NAMES asterix
    PATHS
        ${CMAKE_SOURCE_DIR}/../../../install/lib
        ${CMAKE_SOURCE_DIR}/../../../../install/lib
        /usr/lib
        /usr/local/lib
)

find_path(ASTERIX_INCLUDE_DIR
    NAMES WiresharkWrapper.h
    PATHS
        ${CMAKE_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/../../../install/include/asterix
        ${CMAKE_SOURCE_DIR}/../../../../install/include/asterix
        /usr/include/asterix
        /usr/local/include/asterix
)

if(NOT ASTERIX_LIBRARY OR NOT ASTERIX_INCLUDE_DIR)
    message(WARNING "ASTERIX library not found. Build the main project first.")
    message(STATUS "ASTERIX_LIBRARY: ${ASTERIX_LIBRARY}")
    message(STATUS "ASTERIX_INCLUDE_DIR: ${ASTERIX_INCLUDE_DIR}")
    return()
endif()

message(STATUS "ASTERIX library: ${ASTERIX_LIBRARY}")
message(STATUS "ASTERIX include: ${ASTERIX_INCLUDE_DIR}")

# Plugin source files
set(PLUGIN_SOURCES
    packet-asterix.c
)

# Create plugin shared library
add_library(asterix MODULE ${PLUGIN_SOURCES})

# Set plugin properties
set_target_properties(asterix PROPERTIES
    PREFIX ""
    OUTPUT_NAME "asterix"
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(asterix PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}     # For local config.h
    ${WIRESHARK_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${ASTERIX_INCLUDE_DIR}
)

# Link libraries
target_link_directories(asterix PRIVATE
    ${WIRESHARK_LIBRARY_DIRS}
    ${GLIB_LIBRARY_DIRS}
)

target_link_libraries(asterix PRIVATE
    ${ASTERIX_LIBRARY}
    ${GLIB_LIBRARIES}
    wireshark
    wsutil
)

# Compiler definitions
target_compile_definitions(asterix PRIVATE
    HAVE_PLUGINS
    WIRESHARK_WRAPPER
    $<$<CONFIG:Debug>:DEBUG>
)

# Compiler warnings
target_compile_options(asterix PRIVATE
    $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<C_COMPILER_ID:MSVC>:/W4>
)

# Determine plugin installation directory
if(NOT DEFINED WIRESHARK_PLUGIN_DIR)
    # Get Wireshark version for plugin path
    execute_process(
        COMMAND wireshark --version
        OUTPUT_VARIABLE WIRESHARK_VERSION_OUTPUT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(WIRESHARK_VERSION_OUTPUT MATCHES "Wireshark ([0-9]+)\\.([0-9]+)")
        set(WS_MAJOR ${CMAKE_MATCH_1})
        set(WS_MINOR ${CMAKE_MATCH_2})
        set(WS_VERSION "${WS_MAJOR}.${WS_MINOR}")
    else()
        set(WS_VERSION "4.4")  # Default to current stable
    endif()

    # Standard plugin paths
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(WIRESHARK_PLUGIN_DIR "$ENV{HOME}/.local/lib/wireshark/plugins/${WS_VERSION}/epan")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(WIRESHARK_PLUGIN_DIR "$ENV{HOME}/Library/Application Support/Wireshark/plugins/${WS_VERSION}/epan")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(WIRESHARK_PLUGIN_DIR "$ENV{APPDATA}/Wireshark/plugins/${WS_VERSION}/epan")
    else()
        set(WIRESHARK_PLUGIN_DIR "${CMAKE_INSTALL_LIBDIR}/wireshark/plugins/${WS_VERSION}/epan")
    endif()
endif()

message(STATUS "Plugin will be installed to: ${WIRESHARK_PLUGIN_DIR}")

# Installation
install(TARGETS asterix
    LIBRARY DESTINATION ${WIRESHARK_PLUGIN_DIR}
    RUNTIME DESTINATION ${WIRESHARK_PLUGIN_DIR}
)

# Also install ASTERIX config files
install(DIRECTORY ${CMAKE_SOURCE_DIR}/../../../../asterix/config/
    DESTINATION share/asterix/config
    FILES_MATCHING
    PATTERN "*.xml"
    PATTERN "*.ini"
)

# Summary
message(STATUS "")
message(STATUS "ASTERIX Wireshark Plugin Configuration:")
message(STATUS "  Wireshark version: ${WS_VERSION}")
message(STATUS "  Plugin directory: ${WIRESHARK_PLUGIN_DIR}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
