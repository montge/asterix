#
#  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
#
#  This file is part of Asterix.
#
#  Asterix is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Asterix is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
#
#
# AUTHOR:  Damir Salantic, Croatia Control Ltd.
# ENHANCED: 2025-10-17 for DO-278A AL-3 compliance and multi-platform packaging

cmake_minimum_required(VERSION 3.20)

# Read version from VERSION file
# Supports key=value format with CPP_VERSION or PROJECT_VERSION
file(READ "${CMAKE_SOURCE_DIR}/VERSION" VERSION_FILE_CONTENT)
string(STRIP "${VERSION_FILE_CONTENT}" VERSION_FILE_CONTENT)

# Extract CPP_VERSION or PROJECT_VERSION (whitespace-insensitive)
string(REGEX MATCH "CPP_VERSION[ \t]*=[ \t]*([0-9]+\\.[0-9]+\\.[0-9]+)" CPP_MATCH "${VERSION_FILE_CONTENT}")
if(CPP_MATCH)
    set(PROJECT_VERSION_STRING ${CMAKE_MATCH_1})
else()
    # Fallback to PROJECT_VERSION
    string(REGEX MATCH "PROJECT_VERSION[ \t]*=[ \t]*([0-9]+\\.[0-9]+\\.[0-9]+)" PROJ_MATCH "${VERSION_FILE_CONTENT}")
    if(PROJ_MATCH)
        set(PROJECT_VERSION_STRING ${CMAKE_MATCH_1})
    else()
        # Legacy format: single line with just version number
        file(STRINGS "${CMAKE_SOURCE_DIR}/VERSION" PROJECT_VERSION_STRING LIMIT_COUNT 1)
        string(STRIP "${PROJECT_VERSION_STRING}" PROJECT_VERSION_STRING)
    endif()
endif()

# Parse version components (MAJOR.MINOR.PATCH)
string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" VERSION_MATCH "${PROJECT_VERSION_STRING}")
if(NOT VERSION_MATCH)
    message(FATAL_ERROR "Invalid version format in VERSION file: ${PROJECT_VERSION_STRING}. Expected format: MAJOR.MINOR.PATCH")
endif()

set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
set(PROJECT_VERSION ${PROJECT_VERSION_STRING})

# Project version - read from VERSION file
project(asterix
    VERSION ${PROJECT_VERSION_STRING}
    DESCRIPTION "ASTERIX protocol decoder library and tools"
    LANGUAGES C CXX)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_EXECUTABLE "Build command-line executable" ON)
option(BUILD_TESTING "Build tests" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_ZEROMQ "Enable ZeroMQ transport support" OFF)
option(ENABLE_MQTT "Enable MQTT transport support" OFF)
option(ENABLE_GRPC "Enable gRPC transport support" OFF)
option(ENABLE_CYCLONEDDS "Enable Cyclone DDS transport support" OFF)

# C++ standard: C++23 for GCC/Clang, C++20 for MSVC (MSVC doesn't fully support C++23 yet)
if(MSVC)
    # MSVC: Use C++20 (latest fully supported standard)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    # Use /std:c++20 for MSVC
    string(REGEX REPLACE "/std:c\\+\\+[^ ]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
    
    # C standard: MSVC typically uses C11/C17
    set(CMAKE_C_STANDARD 17)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)
    
    if(MSVC_VERSION LESS 1920)  # VS 2019 v16.0
        message(WARNING
            "MSVC 2019 v16.0+ (version 1920) is recommended for C++20 support. "
            "You are using MSVC ${MSVC_VERSION}.")
    endif()
else()
    # GCC/Clang: Use C++23
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_C_STANDARD 23)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)
endif()

# Compiler Version Checking for C++23 (non-MSVC)
if(NOT MSVC)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
            message(WARNING
                "GCC 13.0+ is recommended for full C++23 support. "
                "You are using GCC ${CMAKE_CXX_COMPILER_VERSION}. "
                "Some C++23 features may not work correctly.")
        endif()
        # Enable specific C++23 flags if needed
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
            message(WARNING
                "Clang 16.0+ is recommended for full C++23 support. "
                "You are using Clang ${CMAKE_CXX_COMPILER_VERSION}.")
        endif()
        # Clang needs libc++ for full C++23 support on non-Apple platforms
        if(NOT APPLE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
        endif()

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
            message(WARNING
                "AppleClang 15.0+ is recommended for full C++23 support. "
                "You are using AppleClang ${CMAKE_CXX_COMPILER_VERSION}.")
        endif()
    endif()
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# macOS RPATH configuration
# Enable use of @rpath in install names
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    # Set RPATH to look for libraries relative to the executable
    # @loader_path/../lib means "look in lib directory relative to the executable"
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
    # Also add install prefix lib directory for development builds
    list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    # Don't skip the full RPATH for the build tree (for testing)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    # When building, use the install RPATH already (for testing before install)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    # The RPATH to be used when installing
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/asterix
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/main
)

# Find dependencies
include(FindEXPAT)
find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

# Optional ZeroMQ support
if(ENABLE_ZEROMQ)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(ZMQ libzmq)
    endif()

    if(NOT ZMQ_FOUND)
        # Try manual search
        find_path(ZMQ_INCLUDE_DIRS zmq.h
            PATHS /usr/include /usr/local/include
        )
        find_library(ZMQ_LIBRARIES zmq
            PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
        )
        if(ZMQ_INCLUDE_DIRS AND ZMQ_LIBRARIES)
            set(ZMQ_FOUND TRUE)
        endif()
    endif()

    if(ZMQ_FOUND)
        message(STATUS "ZeroMQ found: ${ZMQ_LIBRARIES}")
        include_directories(${ZMQ_INCLUDE_DIRS})
        add_compile_definitions(HAVE_ZEROMQ)
    else()
        message(WARNING "ZeroMQ requested but not found. ZeroMQ support disabled.")
        set(ENABLE_ZEROMQ OFF)
    endif()
endif()

# Optional MQTT support (requires libmosquitto)
if(ENABLE_MQTT)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(MOSQUITTO libmosquitto)
    endif()

    if(NOT MOSQUITTO_FOUND)
        # Try manual search
        find_path(MOSQUITTO_INCLUDE_DIRS mosquitto.h
            PATHS /usr/include /usr/local/include
        )
        find_library(MOSQUITTO_LIBRARIES mosquitto
            PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
        )
        if(MOSQUITTO_INCLUDE_DIRS AND MOSQUITTO_LIBRARIES)
            set(MOSQUITTO_FOUND TRUE)
        endif()
    endif()

    if(MOSQUITTO_FOUND)
        message(STATUS "Mosquitto (MQTT) found: ${MOSQUITTO_LIBRARIES}")
        include_directories(${MOSQUITTO_INCLUDE_DIRS})
        add_compile_definitions(HAVE_MQTT)
    else()
        message(WARNING "MQTT requested but libmosquitto not found. MQTT support disabled.")
        set(ENABLE_MQTT OFF)
    endif()
endif()

# Optional gRPC support (requires grpc++)
if(ENABLE_GRPC)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GRPC grpc++)
    endif()

    if(NOT GRPC_FOUND)
        # Try CMake find_package for gRPC
        find_package(gRPC CONFIG QUIET)
        if(gRPC_FOUND)
            set(GRPC_FOUND TRUE)
            set(GRPC_LIBRARIES gRPC::grpc++)
        else()
            # Try manual search
            find_path(GRPC_INCLUDE_DIRS grpcpp/grpcpp.h
                PATHS /usr/include /usr/local/include
            )
            find_library(GRPC_LIBRARIES grpc++
                PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
            )
            if(GRPC_INCLUDE_DIRS AND GRPC_LIBRARIES)
                set(GRPC_FOUND TRUE)
            endif()
        endif()
    endif()

    if(GRPC_FOUND)
        message(STATUS "gRPC found: ${GRPC_LIBRARIES}")
        include_directories(${GRPC_INCLUDE_DIRS})
        add_compile_definitions(HAVE_GRPC)
    else()
        message(WARNING "gRPC requested but not found. gRPC support disabled.")
        set(ENABLE_GRPC OFF)
    endif()
endif()

# Optional Cyclone DDS support (requires CycloneDDS C++ bindings)
if(ENABLE_CYCLONEDDS)
    find_package(CycloneDDS-CXX CONFIG QUIET)
    if(CycloneDDS-CXX_FOUND)
        set(CYCLONEDDS_FOUND TRUE)
        set(CYCLONEDDS_LIBRARIES CycloneDDS-CXX::ddscxx)
        message(STATUS "Cyclone DDS C++ found via CMake config")
    else()
        # Try pkg-config
        find_package(PkgConfig)
        if(PkgConfig_FOUND)
            pkg_check_modules(CYCLONEDDS cyclonedds-cxx)
        endif()

        if(NOT CYCLONEDDS_FOUND)
            # Try manual search
            find_path(CYCLONEDDS_INCLUDE_DIRS dds/dds.hpp
                PATHS /usr/include /usr/local/include
            )
            find_library(CYCLONEDDS_LIBRARIES ddscxx
                PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
            )
            if(CYCLONEDDS_INCLUDE_DIRS AND CYCLONEDDS_LIBRARIES)
                set(CYCLONEDDS_FOUND TRUE)
            endif()
        endif()
    endif()

    if(CYCLONEDDS_FOUND)
        message(STATUS "Cyclone DDS found: ${CYCLONEDDS_LIBRARIES}")
        if(CYCLONEDDS_INCLUDE_DIRS)
            include_directories(${CYCLONEDDS_INCLUDE_DIRS})
        endif()
        add_compile_definitions(HAVE_CYCLONEDDS)
    else()
        message(WARNING "Cyclone DDS requested but not found. Cyclone DDS support disabled.")
        set(ENABLE_CYCLONEDDS OFF)
    endif()
endif()

# Coverage flags (if enabled)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# SECURITY: Platform-specific compiler and linker hardening flags
if(MSVC)
    # MSVC: Security Development Lifecycle (SDL) checks and stack protection
    # /GS enables buffer security checks (stack protection)
    # /sdl enables additional security checks
    # /W4 enables high warning level, /WX- makes warnings non-fatal
    add_compile_options(/W4 /WX- /GS /sdl)
    # Suppress common MSVC warnings that don't affect functionality
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)  # Allow POSIX functions like strdup
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang: Stack protection
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")
    
    # SECURITY: Buffer overflow detection (only for Release builds with -O2)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
endif()

# SECURITY: Read-only relocations and immediate binding (Linux only)
if(UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro,-z,now")
endif()

# Library source files
set(ASTERIX_LIB_SOURCES
    # Core ASTERIX parsing
    src/asterix/AsterixData.cpp
    src/asterix/AsterixDefinition.cpp
    src/asterix/Category.cpp
    src/asterix/DataBlock.cpp
    src/asterix/DataItem.cpp
    src/asterix/DataItemBits.cpp
    src/asterix/DataItemDescription.cpp
    src/asterix/DataItemFormat.cpp
    src/asterix/DataItemFormatBDS.cpp
    src/asterix/DataItemFormatCompound.cpp
    src/asterix/DataItemFormatExplicit.cpp
    src/asterix/DataItemFormatFixed.cpp
    src/asterix/DataItemFormatRepetitive.cpp
    src/asterix/DataItemFormatVariable.cpp
    src/asterix/DataRecord.cpp
    src/asterix/InputParser.cpp
    src/asterix/Tracer.cpp
    src/asterix/UAP.cpp
    src/asterix/UAPItem.cpp
    src/asterix/Utils.cpp
    src/asterix/XMLParser.cpp

    # Format handlers
    src/asterix/asterixformat.cxx
    src/asterix/asterixrawsubformat.cxx
    src/asterix/asterixpcapsubformat.cxx
    src/asterix/asterixfinalsubformat.cxx
    src/asterix/asterixhdlcsubformat.cxx
    src/asterix/asterixhdlcparsing.c
    src/asterix/asterixgpssubformat.cxx

    # Engine
    src/engine/globals.cpp
    src/engine/channelfactory.cxx
    src/engine/converterengine.cxx
    src/engine/descriptor.cxx
    src/engine/devicefactory.cxx
    src/engine/diskdevice.cxx
    src/engine/stddevice.cxx
    src/engine/tcpdevice.cxx
    src/engine/udpdevice.cxx
)

# Serial device only supported on POSIX systems (not Windows)
if(NOT WIN32)
    list(APPEND ASTERIX_LIB_SOURCES src/engine/serialdevice.cxx)
endif()

# ZeroMQ device (optional)
if(ENABLE_ZEROMQ AND ZMQ_FOUND)
    list(APPEND ASTERIX_LIB_SOURCES src/engine/zeromqdevice.cxx)
endif()

# MQTT device (optional)
if(ENABLE_MQTT AND MOSQUITTO_FOUND)
    list(APPEND ASTERIX_LIB_SOURCES src/engine/mqttdevice.cxx)
endif()

# gRPC device (optional)
if(ENABLE_GRPC AND GRPC_FOUND)
    list(APPEND ASTERIX_LIB_SOURCES src/engine/grpcdevice.cxx)
endif()

# Cyclone DDS device (optional)
if(ENABLE_CYCLONEDDS AND CYCLONEDDS_FOUND)
    list(APPEND ASTERIX_LIB_SOURCES src/engine/cycloneddsdevice.cxx)
endif()

# Go bindings C wrapper (included in library for cgo)
list(APPEND ASTERIX_LIB_SOURCES src/go/asterix_wrapper.cpp)

# Wireshark plugin wrapper (C API for Wireshark dissector)
list(APPEND ASTERIX_LIB_SOURCES src/asterix/WiresharkWrapper.cpp)

set(ASTERIX_LIB_HEADERS
    src/asterix/AsterixData.h
    src/asterix/AsterixDefinition.h
    src/asterix/Category.h
    src/asterix/DataBlock.h
    src/asterix/DataItem.h
    src/asterix/DataItemBits.h
    src/asterix/DataItemDescription.h
    src/asterix/DataItemFormat.h
    src/asterix/DataItemFormatBDS.h
    src/asterix/DataItemFormatCompound.h
    src/asterix/DataItemFormatExplicit.h
    src/asterix/DataItemFormatFixed.h
    src/asterix/DataItemFormatRepetitive.h
    src/asterix/DataItemFormatVariable.h
    src/asterix/DataRecord.h
    src/asterix/InputParser.h
    src/asterix/Tracer.h
    src/asterix/UAP.h
    src/asterix/UAPItem.h
    src/asterix/Utils.h
    src/asterix/WiresharkWrapper.h
    src/asterix/XMLParser.h
    src/go/asterix.h
)

# Build shared library
if(BUILD_SHARED_LIBS)
    add_library(asterix_shared SHARED ${ASTERIX_LIB_SOURCES})
    set_target_properties(asterix_shared PROPERTIES
        OUTPUT_NAME asterix
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER "${ASTERIX_LIB_HEADERS}"
    )
    # Enable Wireshark wrapper API in the shared library
    target_compile_definitions(asterix_shared PRIVATE WIRESHARK_WRAPPER)
    # On macOS, set the install name to use @rpath
    if(APPLE)
        set_target_properties(asterix_shared PROPERTIES
            INSTALL_NAME_DIR "@rpath"
        )
    endif()
    target_link_libraries(asterix_shared ${EXPAT_LIBRARIES})

    # Link ZeroMQ if enabled
    if(ENABLE_ZEROMQ AND ZMQ_FOUND)
        target_link_libraries(asterix_shared ${ZMQ_LIBRARIES})
    endif()

    # Link MQTT if enabled
    if(ENABLE_MQTT AND MOSQUITTO_FOUND)
        target_link_libraries(asterix_shared ${MOSQUITTO_LIBRARIES})
    endif()

    # Link gRPC if enabled
    if(ENABLE_GRPC AND GRPC_FOUND)
        target_link_libraries(asterix_shared ${GRPC_LIBRARIES})
    endif()

    # Link Cyclone DDS if enabled
    if(ENABLE_CYCLONEDDS AND CYCLONEDDS_FOUND)
        target_link_libraries(asterix_shared ${CYCLONEDDS_LIBRARIES})
    endif()

    # Install shared library
    # On macOS, LIBRARY DESTINATION installs .dylib files
    # For versioned libraries, CMake automatically handles the symlinks
    install(TARGETS asterix_shared
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/asterix
    )
endif()

# Build static library
if(BUILD_STATIC_LIBS)
    add_library(asterix_static STATIC ${ASTERIX_LIB_SOURCES})
    set_target_properties(asterix_static PROPERTIES
        OUTPUT_NAME asterix
        VERSION ${PROJECT_VERSION}
        PUBLIC_HEADER "${ASTERIX_LIB_HEADERS}"
    )
    # Enable Wireshark wrapper API in the static library
    target_compile_definitions(asterix_static PRIVATE WIRESHARK_WRAPPER)
    target_link_libraries(asterix_static ${EXPAT_LIBRARIES})

    # Link ZeroMQ if enabled
    if(ENABLE_ZEROMQ AND ZMQ_FOUND)
        target_link_libraries(asterix_static ${ZMQ_LIBRARIES})
    endif()

    # Link MQTT if enabled
    if(ENABLE_MQTT AND MOSQUITTO_FOUND)
        target_link_libraries(asterix_static ${MOSQUITTO_LIBRARIES})
    endif()

    # Link gRPC if enabled
    if(ENABLE_GRPC AND GRPC_FOUND)
        target_link_libraries(asterix_static ${GRPC_LIBRARIES})
    endif()

    # Link Cyclone DDS if enabled
    if(ENABLE_CYCLONEDDS AND CYCLONEDDS_FOUND)
        target_link_libraries(asterix_static ${CYCLONEDDS_LIBRARIES})
    endif()

    # Install static library
    # ARCHIVE DESTINATION installs .a files on all platforms including macOS
    install(TARGETS asterix_static
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/asterix
    )
endif()

# Generate version.h from template
configure_file(
    "${CMAKE_SOURCE_DIR}/src/main/version.h.in"
    "${CMAKE_BINARY_DIR}/version.h"
    @ONLY
)

# Build executable (CLI)
if(BUILD_EXECUTABLE)
    add_executable(asterix_exe
        src/main/asterix.cpp
        src/main/asterix.h
    )
    
    # Use generated version.h from build directory
    target_include_directories(asterix_exe PRIVATE ${CMAKE_BINARY_DIR})

    set_target_properties(asterix_exe PROPERTIES
        OUTPUT_NAME asterix
    )

    # Link with shared library if available, otherwise static
    if(BUILD_SHARED_LIBS)
        target_link_libraries(asterix_exe asterix_shared)
        # On macOS, ensure executable has RPATH to find shared library
        if(APPLE)
            set_target_properties(asterix_exe PROPERTIES
                INSTALL_RPATH "@loader_path/../lib"
            )
        endif()
    elseif(BUILD_STATIC_LIBS)
        target_link_libraries(asterix_exe asterix_static)
    else()
        message(FATAL_ERROR "Must build either shared or static library to build executable")
    endif()

    # Install executable
    install(TARGETS asterix_exe
        RUNTIME DESTINATION bin
    )

    # On Windows with vcpkg, install required DLLs
    if(WIN32 AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
        # Find expat DLL using find_file
        find_file(EXPAT_DLL_RELEASE
            NAMES libexpat.dll
            PATHS ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin
            NO_DEFAULT_PATH
        )

        if(EXPAT_DLL_RELEASE)
            install(FILES "${EXPAT_DLL_RELEASE}"
                DESTINATION bin
                CONFIGURATIONS Release RelWithDebInfo MinSizeRel
            )
        endif()

        # For Debug builds, install debug version if it exists
        find_file(EXPAT_DLL_DEBUG
            NAMES libexpatd.dll
            PATHS ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin
            NO_DEFAULT_PATH
        )

        if(EXPAT_DLL_DEBUG)
            install(FILES "${EXPAT_DLL_DEBUG}"
                DESTINATION bin
                CONFIGURATIONS Debug
            )
        endif()
    endif()
endif()

# Install configuration files
install(DIRECTORY asterix/config/
    DESTINATION share/asterix/config
    FILES_MATCHING PATTERN "*.xml" PATTERN "*.dtd" PATTERN "*.ini"
)

# Create symlinks for backwards compatibility with Makefile-based test.sh
# Tests expect config at install/config/ and sample_data at install/sample_data/
# Skip symlink creation during packaging (CMAKE_INSTALL_PREFIX=/usr requires root)
install(CODE "
    # Use DESTDIR if set (for packaging), otherwise use CMAKE_INSTALL_PREFIX
    if(DEFINED ENV{DESTDIR})
        set(INSTALL_ROOT \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}\")
    else()
        set(INSTALL_ROOT \"\${CMAKE_INSTALL_PREFIX}\")
    endif()

    # Skip if installing to system directories during packaging
    # (CPack sets CMAKE_INSTALL_PREFIX=/usr which needs root)
    if(NOT \"\${CMAKE_INSTALL_PREFIX}\" STREQUAL \"/usr\")
        # Remove any existing files/directories before creating symlinks
        file(REMOVE_RECURSE \"\${INSTALL_ROOT}/config\")
        file(REMOVE_RECURSE \"\${INSTALL_ROOT}/sample_data\")
        file(REMOVE_RECURSE \"\${INSTALL_ROOT}/sample_output\")

        # Create symlinks
        file(CREATE_LINK \"\${INSTALL_ROOT}/share/asterix/config\" \"\${INSTALL_ROOT}/config\" SYMBOLIC)
        file(CREATE_LINK \"\${INSTALL_ROOT}/share/asterix/samples\" \"\${INSTALL_ROOT}/sample_data\" SYMBOLIC)
        file(CREATE_LINK \"\${INSTALL_ROOT}/share/asterix/sample_output\" \"\${INSTALL_ROOT}/sample_output\" SYMBOLIC)
    else()
        message(STATUS \"Skipping symlink creation for system install prefix: \${CMAKE_INSTALL_PREFIX}\")
    endif()
")

# Install sample data (optional)
install(DIRECTORY asterix/sample_data/
    DESTINATION share/asterix/samples
    FILES_MATCHING PATTERN "*"
)

# Install sample output files (for test comparisons)
install(DIRECTORY asterix/sample_output/
    DESTINATION share/asterix/sample_output
    FILES_MATCHING PATTERN "*.txt" PATTERN "*.xml"
)

# Install documentation
install(FILES
    README.md
    CLAUDE.md
    DO-278_IMPLEMENTATION_GUIDE.md
    DESTINATION share/doc/asterix
)

# Testing (if enabled)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests/cpp)
endif()

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "asterix")
set(CPACK_PACKAGE_VENDOR "Croatia Control Ltd.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ASTERIX protocol decoder")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "https://github.com/montge/asterix")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# DEB package configuration
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ASTERIX Maintainers")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libexpat1 (>= 2.1)")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/montge/asterix")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# RPM package configuration
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
set(CPACK_RPM_PACKAGE_REQUIRES "expat >= 2.1")
set(CPACK_RPM_PACKAGE_URL "https://github.com/montge/asterix")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

# Package generators
set(CPACK_GENERATOR "TGZ;ZIP")

# Add DEB generator on Debian/Ubuntu
if(UNIX AND NOT APPLE)
    if(EXISTS "/etc/debian_version")
        list(APPEND CPACK_GENERATOR "DEB")
    endif()
    if(EXISTS "/etc/redhat-release")
        list(APPEND CPACK_GENERATOR "RPM")
    endif()
endif()

include(CPack)

# Print build configuration
message(STATUS "")
message(STATUS "ASTERIX ${PROJECT_VERSION} Build Configuration:")
message(STATUS "  Build shared library: ${BUILD_SHARED_LIBS}")
message(STATUS "  Build static library: ${BUILD_STATIC_LIBS}")
message(STATUS "  Build executable: ${BUILD_EXECUTABLE}")
message(STATUS "  Build testing: ${BUILD_TESTING}")
message(STATUS "  Enable coverage: ${ENABLE_COVERAGE}")
if(ENABLE_ZEROMQ)
    message(STATUS "  ZeroMQ support: ${ZMQ_FOUND}")
else()
    message(STATUS "  ZeroMQ support: OFF")
endif()
if(ENABLE_MQTT)
    message(STATUS "  MQTT support: ${MOSQUITTO_FOUND}")
else()
    message(STATUS "  MQTT support: OFF")
endif()
if(ENABLE_GRPC)
    message(STATUS "  gRPC support: ${GRPC_FOUND}")
else()
    message(STATUS "  gRPC support: OFF")
endif()
if(ENABLE_CYCLONEDDS)
    message(STATUS "  Cyclone DDS support: ${CYCLONEDDS_FOUND}")
else()
    message(STATUS "  Cyclone DDS support: OFF")
endif()
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Package generators: ${CPACK_GENERATOR}")
message(STATUS "")
