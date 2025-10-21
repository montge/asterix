#
#  ASTERIX Performance Benchmark Framework
#  CMake Build Configuration
#

cmake_minimum_required(VERSION 3.12)

project(asterix_benchmarks
    VERSION 1.0.0
    DESCRIPTION "ASTERIX Performance Benchmarks"
    LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Always use Release builds for benchmarking
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler optimizations for benchmarking
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find dependencies
find_package(Threads REQUIRED)

# Option to use Google Benchmark library
option(USE_GOOGLE_BENCHMARK "Use Google Benchmark library" OFF)

if(USE_GOOGLE_BENCHMARK)
    find_package(benchmark QUIET)
    if(benchmark_FOUND)
        message(STATUS "Google Benchmark found, will be used for statistical analysis")
        add_compile_definitions(HAVE_GOOGLE_BENCHMARK)
    else()
        message(STATUS "Google Benchmark not found, using built-in timing")
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/asterix
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/main
)

# Common header-only library
add_library(benchmark_common INTERFACE)
target_include_directories(benchmark_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================================================
# PCAP Processing Benchmark
# ============================================================================
add_executable(benchmark_pcap_processing
    benchmark_pcap_processing.cpp
)

target_link_libraries(benchmark_pcap_processing
    benchmark_common
    Threads::Threads
)

# ============================================================================
# JSON Output Benchmark
# ============================================================================
add_executable(benchmark_json_output
    benchmark_json_output.cpp
)

target_link_libraries(benchmark_json_output
    benchmark_common
    Threads::Threads
)

# ============================================================================
# UDP Multicast Benchmark
# ============================================================================
add_executable(benchmark_udp_multicast
    benchmark_udp_multicast.cpp
)

target_link_libraries(benchmark_udp_multicast
    benchmark_common
    Threads::Threads
)

# ============================================================================
# Installation
# ============================================================================
install(TARGETS
    benchmark_pcap_processing
    benchmark_json_output
    benchmark_udp_multicast
    RUNTIME DESTINATION bin
)

install(FILES
    README.md
    benchmark_common.h
    DESTINATION share/asterix/benchmarks
)

# ============================================================================
# Testing targets (optional)
# ============================================================================
enable_testing()

# Quick sanity tests to ensure benchmarks run
add_test(NAME benchmark_json_quick
    COMMAND benchmark_json_output --records 100 --iterations 1 --warmup 0
)

# Print build configuration
message(STATUS "")
message(STATUS "ASTERIX Benchmarks Build Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  CXX Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "  Google Benchmark: ${USE_GOOGLE_BENCHMARK}")
message(STATUS "  Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "")
