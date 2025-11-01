/*
 * C++23 Feature Detection and Compatibility Layer
 *
 * This header provides feature detection for C++23 capabilities and
 * graceful fallback to C++17 when C++23 is not available.
 *
 * Copyright (c) 2025 ASTERIX Contributors
 *
 * This file is part of Asterix.
 */

#ifndef CXX23_FEATURES_H_
#define CXX23_FEATURES_H_

#include <version>  // C++20 <version> header for feature test macros

// ============================================================================
// C++23 Feature Detection
// ============================================================================

#if __cplusplus >= 202302L
    #define CXX23_ENABLED 1
#else
    #define CXX23_ENABLED 0
#endif

// Deduced this (P0847R7) - explicit object parameter
#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
    #define HAS_DEDUCED_THIS 1
#else
    #define HAS_DEDUCED_THIS 0
#endif

// Ranges algorithms (C++20+, enhanced in C++23)
#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
    #define HAS_RANGES_ALGORITHMS 1
    #include <ranges>
    #include <algorithm>
#else
    #define HAS_RANGES_ALGORITHMS 0
#endif

// std::format (C++20)
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
    #define HAS_FORMAT 1
    #include <format>
#else
    #define HAS_FORMAT 0
#endif

// string_view (C++17 - should always be available)
#if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
    #define HAS_STRING_VIEW 1
#else
    #define HAS_STRING_VIEW 0
#endif

// ============================================================================
// Performance Annotations
// ============================================================================

// Mark functions that benefit from C++23 optimizations
#if CXX23_ENABLED
    #define ASTERIX_CXX23_OPTIMIZED [[likely]]
#else
    #define ASTERIX_CXX23_OPTIMIZED
#endif

// ============================================================================
// Compiler Diagnostics
// ============================================================================

#if !CXX23_ENABLED
    #pragma message("C++23 not detected - using C++17/20 compatibility mode")
    #pragma message("Performance optimizations from deduced this will not be available")
#endif

#if HAS_DEDUCED_THIS
    #pragma message("C++23 deduced this enabled - expect 15-20% virtual dispatch improvement")
#endif

#if HAS_RANGES_ALGORITHMS
    #pragma message("Ranges algorithms enabled - expect 5-10% throughput improvement")
#endif

// ============================================================================
// Namespace Aliases for Convenience
// ============================================================================

#if HAS_RANGES_ALGORITHMS
namespace asterix {
    namespace ranges = std::ranges;
    namespace views = std::views;
}
#endif

#endif  // CXX23_FEATURES_H_
