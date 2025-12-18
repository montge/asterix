/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

/**
 * @file Tracer.h
 * @brief Debug logging and diagnostics singleton for ASTERIX decoder
 *
 * This file defines the Tracer class which provides a centralized logging
 * system for error reporting and diagnostics throughout the ASTERIX decoder.
 * The singleton pattern ensures consistent logging behavior across all
 * components.
 */

#ifndef TRACER_H_
#define TRACER_H_

#include <stdlib.h>

/**
 * @typedef ptExtPrintf
 * @brief Function pointer type for printf-style logging with return value
 *
 * Used to configure external logging functions that return the number of
 * characters written (like printf).
 *
 * @param format Printf-style format string
 * @param ... Variable arguments matching format specifiers
 * @return Number of characters written, or negative on error
 */
typedef int(*ptExtPrintf)(char const *, ...);

/**
 * @typedef ptExtVoidPrintf
 * @brief Function pointer type for printf-style logging without return value
 *
 * Used to configure external logging functions that don't return a value
 * (e.g., Python logging functions, GUI message boxes).
 *
 * @param format Printf-style format string
 * @param ... Variable arguments matching format specifiers
 */
typedef void(*ptExtVoidPrintf)(char const *, ...);

/**
 * @class Tracer
 * @brief Singleton class for centralized debug logging and error reporting
 *
 * Tracer provides a unified logging interface for the ASTERIX decoder with
 * configurable output destinations and log levels. It supports error reporting,
 * warnings, and debug messages with printf-style formatting.
 *
 * @par Singleton Pattern
 * Tracer uses the singleton pattern to ensure a single global logging instance.
 * Access via Tracer::instance() or static methods like Tracer::Error().
 *
 * @par Log Levels
 * The log level controls which messages are output:
 * - **0 (Silent)**: No output (all messages suppressed)
 * - **1 (Errors Only)**: Only Error() calls are output (default)
 * - **2 (Warnings)**: Error() and Warning() calls (not yet implemented)
 * - **3 (Info)**: Error(), Warning(), and Info() calls (not yet implemented)
 * - **4 (Debug)**: All messages including Debug() calls (not yet implemented)
 *
 * @par Output Destinations
 * By default, Tracer outputs to stdout via puts(). Custom output functions
 * can be configured via Configure():
 * - Python logging integration (ptExtVoidPrintf)
 * - File logging (ptExtPrintf)
 * - GUI message boxes (ptExtVoidPrintf)
 * - Custom diagnostics systems (ptExtPrintf/ptExtVoidPrintf)
 *
 * @par Thread Safety
 * This class is NOT thread-safe. The singleton instance and logging calls
 * are not protected by mutexes. Concurrent access from multiple threads
 * requires external synchronization.
 *
 * @par Memory Management
 * - The singleton instance is created on first use via instance()
 * - Explicitly destroy via Delete() if needed (e.g., during shutdown)
 * - Messages are limited to 1024 bytes (truncated if longer)
 *
 * @par Usage Throughout Codebase
 * Tracer is used extensively for error reporting in:
 * - DataItemFormat* classes (parsing errors)
 * - Category/UAP loading (configuration errors)
 * - XMLParser (XML syntax/validation errors)
 * - Input format handlers (PCAP/HDLC/FINAL parsing errors)
 * - Python/Rust FFI boundaries (validation errors)
 *
 * @par Example Usage
 * @code
 * // Basic error logging with formatting
 * if (data == nullptr) {
 *     Tracer::Error("Invalid data pointer at %s:%d", __FILE__, __LINE__);
 *     return false;
 * }
 *
 * // Logging with context
 * if (offset >= length) {
 *     Tracer::Error("Offset %u exceeds length %zu in category %d",
 *                   offset, length, categoryId);
 *     return false;
 * }
 *
 * // Configure custom output (e.g., Python binding)
 * extern "C" void python_log(const char* msg, ...) {
 *     // Python logging implementation
 * }
 * Tracer::Configure(python_log);
 *
 * // Silence all output (e.g., during testing)
 * Tracer::SetLogLevel(0);
 *
 * // Enable full logging (when implemented)
 * Tracer::SetLogLevel(4);
 *
 * // Cleanup on shutdown
 * Tracer::Delete();
 * @endcode
 *
 * @see DataItemFormatBDS::parse() for error reporting examples
 * @see XMLParser for configuration error reporting
 * @see Category::parse() for parsing error diagnostics
 *
 * @note Currently only Error() is implemented. Warning/Info/Debug levels
 *       are planned for future enhancement.
 *
 * @warning Messages longer than 1024 bytes will be truncated without warning.
 */
class Tracer {
public:
    /**
     * @brief Constructs the Tracer singleton instance
     *
     * Initializes the tracer with default configuration:
     * - Log level set to 1 (errors only)
     * - Output functions set to nullptr (uses default stdout via puts())
     *
     * @note This constructor is called automatically by instance() on first use.
     *       You should not call this directly; use instance() instead.
     *
     * @see instance()
     */
    Tracer();

    /**
     * @brief Logs an error message with printf-style formatting
     *
     * Outputs an error message to the configured output destination. The message
     * is formatted using printf-style format strings and variable arguments.
     * Output is suppressed if log level is 0 (silent).
     *
     * @param format Printf-style format string (e.g., "Error: %s at line %d")
     * @param ... Variable arguments matching the format specifiers
     *
     * @par Behavior
     * - Checks log level: if m_logLevel <= 0, no output is produced
     * - Formats message into 1024-byte buffer (messages truncated if longer)
     * - Routes to configured output function or stdout (default)
     *
     * @par Output Routing
     * 1. If pPrintFunc is set, uses that function (returns int)
     * 2. Else if pPrintFunc2 is set, uses that function (returns void)
     * 3. Otherwise uses puts() to stdout
     *
     * @warning Messages longer than 1024 bytes are truncated without indication.
     *
     * @code
     * // Basic error logging
     * Tracer::Error("Failed to parse data");
     *
     * // With context information
     * Tracer::Error("Invalid offset %u at category %d", offset, cat);
     *
     * // With file/line information
     * Tracer::Error("Null pointer at %s:%d", __FILE__, __LINE__);
     * @endcode
     *
     * @see Configure() to set custom output destination
     * @see SetLogLevel() to control output suppression
     */
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 1, 2)))
#endif
    static void Error(const char *format, ...);

    /**
     * @brief Configures custom output function with return value
     *
     * Sets a custom printf-style logging function that returns the number of
     * characters written. Useful for file logging or systems that need to
     * track output success.
     *
     * @param pFunc Function pointer matching ptExtPrintf signature
     *              (returns int, takes printf-style arguments)
     *
     * @note This overrides any previously configured output function.
     *       To reset to default stdout, call Configure(nullptr).
     *
     * @code
     * // File logging example
     * int file_logger(const char* msg, ...) {
     *     FILE* fp = fopen("asterix.log", "a");
     *     if (!fp) return -1;
     *     va_list args;
     *     va_start(args, msg);
     *     int result = vfprintf(fp, msg, args);
     *     va_end(args);
     *     fclose(fp);
     *     return result;
     * }
     * Tracer::Configure(file_logger);
     * @endcode
     *
     * @see Configure(ptExtVoidPrintf) for void-returning variant
     * @see ptExtPrintf for function signature
     */
    static void Configure(ptExtPrintf pFunc);

    /**
     * @brief Configures custom output function without return value
     *
     * Sets a custom printf-style logging function that does not return a value.
     * Useful for Python logging integration, GUI message boxes, or systems
     * where output success is not tracked.
     *
     * @param pFunc Function pointer matching ptExtVoidPrintf signature
     *              (returns void, takes printf-style arguments)
     *
     * @note This overrides any previously configured output function.
     *       To reset to default stdout, call Configure(nullptr).
     *
     * @code
     * // Python logging integration
     * extern "C" void python_logger(const char* msg, ...) {
     *     PyGILState_STATE gstate = PyGILState_Ensure();
     *     // Forward to Python logging.error()
     *     PyErr_Format(PyExc_RuntimeError, "%s", msg);
     *     PyGILState_Release(gstate);
     * }
     * Tracer::Configure(python_logger);
     *
     * // GUI message box
     * void gui_logger(const char* msg, ...) {
     *     char buffer[1024];
     *     va_list args;
     *     va_start(args, msg);
     *     vsnprintf(buffer, sizeof(buffer), msg, args);
     *     va_end(args);
     *     ShowMessageBox("ASTERIX Error", buffer);
     * }
     * Tracer::Configure(gui_logger);
     * @endcode
     *
     * @see Configure(ptExtPrintf) for int-returning variant
     * @see ptExtVoidPrintf for function signature
     */
    static void Configure(ptExtVoidPrintf pFunc);

    /**
     * @brief Destroys the singleton instance and frees resources
     *
     * Explicitly deletes the global Tracer singleton instance. Useful during
     * application shutdown or when reinitializing the logging system.
     *
     * @par Behavior
     * - Deletes g_TracerInstance if non-null
     * - Sets g_TracerInstance to nullptr
     * - Subsequent calls to instance() will recreate the singleton
     *
     * @warning After calling Delete(), the next call to Error() or instance()
     *          will create a new instance with default configuration. Any
     *          previous Configure() settings are lost.
     *
     * @code
     * // Typical usage during shutdown
     * void cleanup() {
     *     Tracer::Delete();
     *     // Other cleanup...
     * }
     *
     * // Reset to default configuration
     * Tracer::Configure(custom_logger);
     * // ... use custom logger ...
     * Tracer::Delete();  // Reset
     * Tracer::Error("Using default stdout again");
     * @endcode
     *
     * @see instance() for singleton creation
     */
    static void Delete();

    /**
     * @brief Global singleton instance pointer
     *
     * Points to the single Tracer instance. Created on first access via instance().
     * Do not access directly; use instance() instead to ensure initialization.
     *
     * @warning This is public for testing purposes. Production code should use
     *          instance() to ensure proper initialization.
     *
     * @see instance()
     */
    static Tracer *g_TracerInstance;

    /**
     * @brief Returns reference to the singleton Tracer instance
     *
     * Lazy initialization: creates the singleton on first call. Subsequent calls
     * return the same instance.
     *
     * @return Reference to the global Tracer singleton
     *
     * @par Thread Safety
     * NOT thread-safe. If multiple threads call instance() concurrently during
     * first initialization, multiple instances may be created (memory leak).
     * Ensure initialization happens on a single thread.
     *
     * @code
     * // Access singleton (internal use)
     * Tracer& tracer = Tracer::instance();
     * tracer.m_logLevel = 2;
     *
     * // Preferred: use static methods
     * Tracer::SetLogLevel(2);
     * Tracer::Error("Message");
     * @endcode
     *
     * @see Delete() to destroy the singleton
     * @see g_TracerInstance for raw pointer access
     */
    static Tracer &instance();

    /**
     * @brief Sets the log level for filtering messages
     *
     * Controls which messages are output based on severity level:
     * - 0: Silent (all messages suppressed)
     * - 1: Errors only (default)
     * - 2-4: Reserved for Warning/Info/Debug (not yet implemented)
     *
     * @param level Log level (0-4). Values outside this range are accepted
     *              but may cause undefined behavior in future implementations.
     *
     * @note Currently only level 0 (silent) and level 1+ (errors) are functional.
     *       Warning/Info/Debug levels are planned for future enhancement.
     *
     * @code
     * // Silence all output (e.g., during unit tests)
     * Tracer::SetLogLevel(0);
     *
     * // Enable error logging (default)
     * Tracer::SetLogLevel(1);
     *
     * // Enable verbose logging (when implemented)
     * Tracer::SetLogLevel(4);
     * @endcode
     *
     * @see GetLogLevel() to query current level
     * @see Error() for message output behavior
     */
    static void SetLogLevel(int level);

    /**
     * @brief Gets the current log level
     *
     * Returns the configured log level that controls message filtering.
     *
     * @return Current log level (0-4)
     *         - 0: Silent
     *         - 1: Errors only (default)
     *         - 2-4: Reserved for future use
     *
     * @code
     * // Save and restore log level
     * int oldLevel = Tracer::GetLogLevel();
     * Tracer::SetLogLevel(0);  // Silence for testing
     * // ... run tests ...
     * Tracer::SetLogLevel(oldLevel);  // Restore
     * @endcode
     *
     * @see SetLogLevel() to modify the log level
     */
    static int GetLogLevel();

    /**
     * @brief Custom output function with return value (printf-style)
     *
     * Function pointer to external logging function that returns number of
     * characters written. Set via Configure(ptExtPrintf).
     *
     * @see Configure(ptExtPrintf)
     */
    ptExtPrintf pPrintFunc;

    /**
     * @brief Custom output function without return value
     *
     * Function pointer to external logging function that returns void.
     * Set via Configure(ptExtVoidPrintf). Used for Python logging, GUI
     * message boxes, or systems that don't track output success.
     *
     * @see Configure(ptExtVoidPrintf)
     */
    ptExtVoidPrintf pPrintFunc2;

    /**
     * @brief Current log level for message filtering
     *
     * Controls which messages are output:
     * - 0: Silent (all messages suppressed)
     * - 1: Errors only (default)
     * - 2: Warnings (not yet implemented)
     * - 3: Info (not yet implemented)
     * - 4: Debug (not yet implemented)
     *
     * @see SetLogLevel() to modify
     * @see GetLogLevel() to query
     */
    int m_logLevel;
};

#endif /* TRACER_H_ */
