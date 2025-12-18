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

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
  #include <winsock2.h>
  #include <time.h>
  #include <io.h>
  #include <process.h>
  #define close _close
  #define read _read
  #define write _write
  #define getpid _getpid

  // Windows compatibility layer for POSIX time functions
  typedef unsigned long useconds_t;

  inline int gettimeofday(struct timeval* tp, void* tzp) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    unsigned long long t = (static_cast<unsigned long long>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    t -= 116444736000000000ULL;
    t /= 10;

    tp->tv_sec = static_cast<long>(t / 1000000UL);
    tp->tv_usec = static_cast<long>(t % 1000000UL);
    return 0;
  }

  inline void usleep(useconds_t usec) {
    Sleep(static_cast<DWORD>(usec / 1000));
  }
#else
  #include <sys/time.h>
  #include <unistd.h>
#endif
#include "asterix.h"
#include "asterixformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixfinalsubformat.hxx"

#include "AsterixDefinition.h"
#include "InputParser.h"

extern bool gSynchronous;

bool CAsterixFinalSubformat::ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, [[maybe_unused]] bool &discard) {
    struct sFinalRecordHeader finalRecordHeader;
    char padding[4];
    static unsigned long lastFileTimeMSec = 0;
    static unsigned long lastMyTimeMSec = 0;

    auto &Descriptor = static_cast<CAsterixFormatDescriptor &>(formatDescriptor);

    // Read final record header
    if (!device.Read(&finalRecordHeader, sizeof(finalRecordHeader))) {
        LOGERROR(1, "Couldn't read packet header.\n");
        return false;
    }

    unsigned long nTimestamp = static_cast<unsigned long>(finalRecordHeader.m_nTimeMMSB) << 16;
    nTimestamp |= static_cast<unsigned long>(finalRecordHeader.m_nTimeMSB) << 8;
    nTimestamp |= static_cast<unsigned long>(finalRecordHeader.m_nTimeLSB);
    // convert timestamp to milliseconds (resolution is 10ms in final format)
    nTimestamp = nTimestamp * 10;

    unsigned int neededLen = finalRecordHeader.m_nByteCountMSB;
    neededLen <<= 8;
    neededLen |= finalRecordHeader.m_nByteCountLSB;

    neededLen -= sizeof(finalRecordHeader); // minus header
    neededLen -= sizeof(padding); // minus padding

    if (neededLen <= 0) {
        LOGERROR(1, "Wrong final record header.\n");
        return false;
    }

    if (gSynchronous) { // In synchronous mode make delays between packets to simulate real tempo
        struct timeval currTime;
        if (gettimeofday(&currTime, nullptr) == 0) {
            unsigned int currTimeMsec = currTime.tv_sec * 1000 + currTime.tv_usec / 1000;
            if (lastMyTimeMSec != 0 && lastFileTimeMSec != 0) {
                unsigned int diffFile = nTimestamp - lastFileTimeMSec;
                unsigned int diffMy = currTimeMsec - lastMyTimeMSec;

                if (diffFile > diffMy) { // sleep for a time difference
                    usleep((diffFile - diffMy) * 1000);
                }
            }

            lastFileTimeMSec = nTimestamp;
            lastMyTimeMSec = currTimeMsec;
        }
    }

    unsigned char *pBuffer = Descriptor.GetNewBuffer(neededLen);

    // Read packet
    if (!device.Read(pBuffer, neededLen)) {
        LOGERROR(1, "Couldn't read packet.\n");
        return false;
    }

    // Read padding bytes
    if (!device.Read(padding, sizeof(padding))) {
        LOGERROR(1, "Couldn't read packet padding.\n");
        return false;
    }

    // parse packet
    if (Descriptor.m_pAsterixData)
        delete Descriptor.m_pAsterixData;

    Descriptor.m_pAsterixData = Descriptor.m_InputParser.parsePacket(pBuffer, neededLen, nTimestamp);

    return true;
}

bool CAsterixFinalSubformat::WritePacket([[maybe_unused]] CBaseFormatDescriptor &formatDescriptor, [[maybe_unused]] CBaseDevice &device, [[maybe_unused]] bool &discard) {
    return false; //TODO
}

bool
CAsterixFinalSubformat::ProcessPacket([[maybe_unused]] CBaseFormatDescriptor &formatDescriptor, [[maybe_unused]] CBaseDevice &device, [[maybe_unused]] bool &discard) {
    return true;
}

bool CAsterixFinalSubformat::Heartbeat([[maybe_unused]] CBaseFormatDescriptor &formatDescriptor, [[maybe_unused]] CBaseDevice &device) {
    return true;
}

