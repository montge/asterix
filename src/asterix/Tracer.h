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

#ifndef TRACER_H_
#define TRACER_H_

#include <stdlib.h>

//! Function pointer type for printf-style logging with return value
typedef int(*ptExtPrintf)(char const *, ...);

//! Function pointer type for printf-style logging without return value
typedef void(*ptExtVoidPrintf)(char const *, ...);

/**
 * @class Tracer
 * @brief Global logging and error reporting facility (singleton)
 *
 * Tracer provides centralized error and diagnostic message logging for the
 * entire ASTERIX parser. It supports both built-in logging to stderr and
 * custom external logging functions (useful for language bindings).
 *
 * ## Design Pattern
 *
 * Tracer is a **singleton** - only one instance exists globally. Access via:
 * ```cpp
 * Tracer::Error("Parse error: %s", errorMsg);  // Static method
 * ```
 *
 * ## Log Levels
 *
 * - **0**: Silent (no output)
 * - **1**: Errors only (default)
 * - **2**: Warnings
 * - **3**: Info
 * - **4**: Debug (verbose)
 *
 * ## Custom Logging Integration
 *
 * Language bindings (Python, Rust, Node.js) can redirect output:
 * ```cpp
 * // Redirect to Python logging
 * Tracer::Configure(my_python_logger);
 * ```
 *
 * ## Usage Example
 *
 * ```cpp
 * // Set log level
 * Tracer::SetLogLevel(1);  // Errors only
 *
 * // Log error message
 * Tracer::Error("Invalid ASTERIX category: %d", category);
 *
 * // Custom handler (Python binding example)
 * int python_logger(const char* fmt, ...) {
 *     va_list args;
 *     va_start(args, fmt);
 *     PyErr_VFormat(fmt, args);
 *     va_end(args);
 *     return 0;
 * }
 * Tracer::Configure(python_logger);
 * ```
 *
 * ## Thread Safety
 *
 * Tracer is **NOT thread-safe**. The singleton is shared across all threads
 * without synchronization. Concurrent logging may interleave messages.
 *
 * @note Default behavior: Prints to stderr
 * @note Singleton lifetime: Exists for entire program duration
 * @note Custom logging functions must be printf-compatible (varargs)
 *
 * @see AsterixDefinition (another singleton in the system)
 */
class Tracer {
public:
    /**
     * @brief Construct the Tracer singleton
     * @note Usually called automatically on first use - don't call directly
     */
    Tracer();

    /**
     * @brief Log an error message (printf-style)
     * @param format Printf-style format string
     * @param ... Variable arguments for format string
     * @note Respects log level setting (requires level >= 1)
     * @note Thread-safe at printf level, but messages may interleave
     */
    static void Error(const char *format, ...);

    /**
     * @brief Configure custom logging function (returns int)
     * @param pFunc Function pointer to printf-compatible logger
     * @note Use for C bindings or external loggers with return values
     */
    static void Configure(ptExtPrintf pFunc);

    /**
     * @brief Configure custom logging function (returns void)
     * @param pFunc Function pointer to printf-compatible logger
     * @note Use for simpler external loggers without return values
     */
    static void Configure(ptExtVoidPrintf pFunc);

    /**
     * @brief Delete the singleton instance
     * @note Rarely needed - singleton persists for program lifetime
     * @warning Calling this while parsers are active causes crashes
     */
    static void Delete();

    static Tracer *g_TracerInstance; //!< Global singleton instance pointer

    /**
     * @brief Get reference to singleton instance
     * @return Reference to the global Tracer instance
     * @note Creates instance on first call (lazy initialization)
     */
    static Tracer &instance();

    /**
     * @brief Set global logging verbosity level
     * @param level Log level (0=silent, 1=errors, 2=warnings, 3=info, 4=debug)
     */
    static void SetLogLevel(int level);

    /**
     * @brief Get current logging verbosity level
     * @return Current log level (0-4)
     */
    static int GetLogLevel();

    ptExtPrintf pPrintFunc; //!< Custom logging function (int return type)
    ptExtVoidPrintf pPrintFunc2; //!< Custom logging function (void return type)
    int m_logLevel;  //!< Current log level (0=silent, 1=errors, 2=warnings, 3=info, 4=debug)
};

#endif /* TRACER_H_ */
