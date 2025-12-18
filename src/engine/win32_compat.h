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
  // Prevent windows.h from including winsock.h (we use winsock2.h instead)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX  // Prevent min/max macro definitions
  #endif

  // Include winsock2.h BEFORE windows.h to ensure we get the right socket API
  // This also provides struct timeval definition needed by gettimeofday()
  #include <winsock2.h>
  #include <ws2tcpip.h>

  #include <windows.h>
  #include <BaseTsd.h>
  #include <io.h>
  #include <cstdarg>  // For va_list in fcntl wrapper

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

  // fcntl constants
  #ifndef F_GETFL
    #define F_GETFL 0
  #endif
  #ifndef F_SETFL
    #define F_SETFL 1
  #endif
  #ifndef FNDELAY
    #define FNDELAY 0
  #endif
  #ifndef O_NONBLOCK
    // O_NONBLOCK must be non-zero so that (flags & O_NONBLOCK) works correctly
    #define O_NONBLOCK 1
  #endif
  #ifndef FIONBIO
    #define FIONBIO 0x8004667E  // Windows ioctl command for non-blocking I/O
  #endif

  // fcntl wrapper for Windows - handles F_SETFL with O_NONBLOCK for socket operations
  // Note: This is a minimal implementation that only supports non-blocking socket I/O
  inline int fcntl(int fd, int cmd, ...) {
    if (cmd == F_SETFL) {
      va_list args;
      va_start(args, cmd);
      int flags = va_arg(args, int);
      va_end(args);

      // Set non-blocking mode on socket
      u_long mode = (flags & O_NONBLOCK) ? 1 : 0;
      return ioctlsocket(static_cast<SOCKET>(fd), FIONBIO, &mode);
    }
    // F_GETFL returns 0 (no flags) on Windows
    return 0;
  }

  // Socket data casting helpers
  // Windows Winsock API requires char* pointers, POSIX uses void*
  #define RECVFROM_CAST(ptr) ((char*)(ptr))
  #define SENDTO_CAST(ptr) ((const char*)(ptr))
  #define SETSOCKOPT_CAST(ptr) ((const char*)(ptr))

  // Note: struct timeval is already defined in winsock2.h on Windows
  // No need to redefine it here

  // gettimeofday implementation for Windows
  // winsock2.h provides struct timeval but not gettimeofday()
  inline int gettimeofday(struct timeval* tp, void* tzp) {
      (void)tzp; // Unused
      FILETIME ft;
      GetSystemTimeAsFileTime(&ft);

      // Convert FILETIME to Unix epoch (microseconds since 1970)
      unsigned long long t = (static_cast<unsigned long long>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
      t -= 116444736000000000ULL; // Convert from Windows epoch to Unix epoch
      t /= 10; // Convert 100-nanosecond intervals to microseconds

      tp->tv_sec = static_cast<long>(t / 1000000UL);
      tp->tv_usec = static_cast<long>(t % 1000000UL);
      return 0;
  }

#else
  // POSIX systems don't need casting or timeval definitions
  #define RECVFROM_CAST(ptr) (ptr)
  #define SENDTO_CAST(ptr) (ptr)
  #define SETSOCKOPT_CAST(ptr) (ptr)
#endif

#endif // WIN32_COMPAT_H
