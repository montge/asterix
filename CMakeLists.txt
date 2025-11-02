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

# Project version - update for releases
project(asterix
    VERSION 2.8.9
    DESCRIPTION "ASTERIX protocol decoder library and tools"
    LANGUAGES C CXX)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_EXECUTABLE "Build command-line executable" ON)
option(BUILD_TESTING "Build tests" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

# C++23 and C23 standards
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler Version Checking for C++23
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

elseif(MSVC)
    if(MSVC_VERSION LESS 1934)  # VS 2022 v17.4
        message(WARNING
            "MSVC 2022 v17.4+ (version 1934) is recommended for C++23 support. "
            "You are using MSVC ${MSVC_VERSION}.")
    endif()
    # Use /std:c++latest for latest C++ standard
    string(REGEX REPLACE "/std:c\\+\\+[^ ]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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

# Coverage flags (if enabled)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# SECURITY: Compiler and linker hardening flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")

# SECURITY: Buffer overflow detection (only for Release builds with -O2)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")

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
    src/engine/channelfactory.cxx
    src/engine/converterengine.cxx
    src/engine/descriptor.cxx
    src/engine/devicefactory.cxx
    src/engine/diskdevice.cxx
    src/engine/serialdevice.cxx
    src/engine/stddevice.cxx
    src/engine/tcpdevice.cxx
    src/engine/udpdevice.cxx
)

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
    src/asterix/XMLParser.h
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
    target_link_libraries(asterix_shared ${EXPAT_LIBRARIES})

    # Install shared library
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
    target_link_libraries(asterix_static ${EXPAT_LIBRARIES})

    # Install static library
    install(TARGETS asterix_static
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/asterix
    )
endif()

# Build executable (CLI)
if(BUILD_EXECUTABLE)
    add_executable(asterix_exe
        src/main/asterix.cpp
        src/main/asterix.h
        src/main/version.h
    )

    set_target_properties(asterix_exe PROPERTIES
        OUTPUT_NAME asterix
    )

    # Link with shared library if available, otherwise static
    if(BUILD_SHARED_LIBS)
        target_link_libraries(asterix_exe asterix_shared)
    elseif(BUILD_STATIC_LIBS)
        target_link_libraries(asterix_exe asterix_static)
    else()
        message(FATAL_ERROR "Must build either shared or static library to build executable")
    endif()

    # Install executable
    install(TARGETS asterix_exe
        RUNTIME DESTINATION bin
    )
endif()

# Install configuration files
install(DIRECTORY asterix/config/
    DESTINATION share/asterix/config
    FILES_MATCHING PATTERN "*.xml" PATTERN "*.dtd" PATTERN "*.ini"
)

# Install sample data (optional)
install(DIRECTORY asterix/sample_data/
    DESTINATION share/asterix/samples
    FILES_MATCHING PATTERN "*"
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
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Package generators: ${CPACK_GENERATOR}")
message(STATUS "")
