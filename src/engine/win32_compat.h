/*
 *  Copyright (c) 2025 ASTERIX Project Contributors
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
 * Windows POSIX Compatibility Layer
 * Provides Windows equivalents for POSIX types, constants, and functions
 */

#ifndef WIN32_COMPAT_H
#define WIN32_COMPAT_H

#ifdef _WIN32
  #include <windows.h>
  #include <BaseTsd.h>
  #include <io.h>

  // POSIX type compatibility
  typedef SSIZE_T ssize_t;
  typedef unsigned long speed_t;  // Serial port baud rate

  // String comparison functions
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp

  // File access constants
  #ifndef R_OK
    #define R_OK 4  // Read permission
  #endif
  #ifndef W_OK
    #define W_OK 2  // Write permission
  #endif
  #ifndef X_OK
    #define X_OK 1  // Execute permission
  #endif
  #ifndef F_OK
    #define F_OK 0  // Existence check
  #endif

  // File descriptor constants
  #ifndef STDIN_FILENO
    #define STDIN_FILENO  0
  #endif
  #ifndef STDOUT_FILENO
    #define STDOUT_FILENO 1
  #endif
  #ifndef STDERR_FILENO
    #define STDERR_FILENO 2
  #endif

  // Path constants
  #ifndef MAXPATHLEN
    #define MAXPATHLEN MAX_PATH
  #endif

  // Socket message flags (no-ops on Windows - Winsock doesn't support these)
  #ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0
  #endif
  #ifndef MSG_WAITALL
    #define MSG_WAITALL 0
  #endif
  #ifndef MSG_DONTWAIT
    #define MSG_DONTWAIT 0
  #endif

  // Function compatibility
  // sleep() takes seconds, Sleep() takes milliseconds
  #define sleep(x) Sleep((x)*1000)

  // fcntl constants (no-ops - Windows uses different APIs)
  #ifndef F_GETFL
    #define F_GETFL 0
  #endif
  #ifndef F_SETFL
    #define F_SETFL 0
  #endif
  #ifndef FNDELAY
    #define FNDELAY 0
  #endif
  #ifndef O_NONBLOCK
    #define O_NONBLOCK 0
  #endif

  // Socket option casting helper
  // Windows setsockopt() requires const char*, POSIX uses void*
  #define SETSOCKOPT_CAST(ptr) ((const char*)(ptr))

#else
  // POSIX systems don't need casting
  #define SETSOCKOPT_CAST(ptr) (ptr)
#endif

#endif // WIN32_COMPAT_H
