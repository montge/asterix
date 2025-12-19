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

  // Implement gettimeofday for Windows
  inline int gettimeofday(struct timeval* tp, void* tzp) {
    // Note: some versions of Windows have gettimeofday in <sys/time.h>
    // but we need to implement it for MSVC
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

  // Implement usleep for Windows (usleep takes microseconds, Sleep takes milliseconds)
  inline void usleep(useconds_t usec) {
    Sleep(static_cast<DWORD>(usec / 1000)); // Convert microseconds to milliseconds
  }
#else
  #include <sys/time.h>
  #include <unistd.h>
#endif

#include "asterix.h"
#include "asterixformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixpcapsubformat.hxx"

#include "AsterixDefinition.h"
#include "InputParser.h"

extern bool gSynchronous;

// Helper: Read and validate PCAP file header
bool CAsterixPcapSubformat::readPcapFileHeader(CAsterixFormatDescriptor &Descriptor, CBaseDevice &device) {
    pcap_hdr_t pcapFileHeader;

    if (!device.Read(&pcapFileHeader, sizeof(pcapFileHeader))) {
        LOGERROR(1, "Couldn't read PCAP file header.\n");
        return false;
    }

    // Determine byte order from magic number
    if (pcapFileHeader.magic_number == 0xA1B2C3D4) {
        Descriptor.m_bInvertByteOrder = false;
    } else if (pcapFileHeader.magic_number == 0xD4C3B2A1) {
        Descriptor.m_bInvertByteOrder = true;
        pcapFileHeader.network = convert_long(pcapFileHeader.network);
    } else {
        LOGERROR(1, "Unknown input file format");
        return false;
    }

    // Determine network type
    if (pcapFileHeader.network == 1) {
        Descriptor.m_ePcapNetworkType = CAsterixFormatDescriptor::NET_ETHERNET;
    } else if (pcapFileHeader.network == 113) {
        Descriptor.m_ePcapNetworkType = CAsterixFormatDescriptor::NET_LINUX;
    } else {
        LOGERROR(1, "Unknown network type in PCAP format");
        return false;
    }

    return true;
}

// Helper: Handle synchronous playback delay
void CAsterixPcapSubformat::handleSynchronousDelay(const pcaprec_hdr_t &recHeader,
                                                   time_t &lastFileTimeSec, useconds_t &lastFileTimeUSec,
                                                   time_t &lastMyTimeSec, useconds_t &lastMyTimeUSec) {
    struct timeval currTime;
    if (gettimeofday(&currTime, nullptr) != 0) {
        return;
    }

    if (lastMyTimeSec != 0 && lastFileTimeSec != 0) {
        unsigned int diffFile = (recHeader.ts_sec - lastFileTimeSec) * 1000000 +
                                recHeader.ts_usec - lastFileTimeUSec;
        unsigned int diffMy = (currTime.tv_sec - lastMyTimeSec) * 1000000 +
                              currTime.tv_usec - lastMyTimeUSec;

        if (diffFile > diffMy) {
            usleep(diffFile - diffMy);
        }
    }

    lastFileTimeSec = recHeader.ts_sec;
    lastFileTimeUSec = recHeader.ts_usec;
    lastMyTimeSec = currTime.tv_sec;
    lastMyTimeUSec = currTime.tv_usec;
}

// Helper: Parse network header (Ethernet or Linux cooked) and return pointer to protocol type
unsigned char* CAsterixPcapSubformat::parseNetworkHeader(unsigned char *pPacketBuffer,
                                                         CAsterixFormatDescriptor &Descriptor,
                                                         bool &bIPInvertByteOrder) {
    unsigned char *pPacketPtr = pPacketBuffer;

    if (Descriptor.m_ePcapNetworkType == CAsterixFormatDescriptor::NET_ETHERNET) {
        pPacketPtr += 12; // Destination MAC (6) + Source MAC (6)
    } else if (Descriptor.m_ePcapNetworkType == CAsterixFormatDescriptor::NET_LINUX) {
        pPacketPtr += 14; // Packet type(2) + Address type(2) + Address length(2) + Source(8)
    }

    unsigned short protoType = *reinterpret_cast<unsigned short *>(pPacketPtr);
    pPacketPtr += 2;

    if (Descriptor.m_bInvertByteOrder) {
        protoType = convert_short(protoType);
    }

    // Determine IP byte order based on protocol type
    bIPInvertByteOrder = Descriptor.m_bInvertByteOrder;
    if (protoType == 0x8) {
        bIPInvertByteOrder = !Descriptor.m_bInvertByteOrder;
    } else if (protoType != 0x800) {
        LOGERROR(1, "Unknown protocol type (%x)\n", protoType);
        return nullptr;
    }

    return pPacketPtr;
}

// Helper: Parse IP header, returns false on failure
bool CAsterixPcapSubformat::parseIPHeader(unsigned char *&pPacketPtr, bool bIPInvertByteOrder,
                                          unsigned short &IPtotalLength) {
    unsigned char IPheaderLength = (*pPacketPtr) & 0x0F;
    pPacketPtr++; // Version + IHL

    pPacketPtr += 1; // TOS

    IPtotalLength = *reinterpret_cast<unsigned short *>(pPacketPtr);
    pPacketPtr += 2;

    if (bIPInvertByteOrder) {
        IPtotalLength = convert_short(IPtotalLength);
    }

    pPacketPtr += 5; // ID(2) + Flags(1) + Fragment Offset(2) + TTL(1) - actually 5 bytes to get to protocol

    unsigned char protocol = *pPacketPtr;
    pPacketPtr++;

    if (protocol != 17) { // Only UDP supported
        LOGERROR(1, "Unsupported protocol type (%x)", protocol);
        return false;
    }

    pPacketPtr += 2; // Checksum
    pPacketPtr += 4; // Source IP
    pPacketPtr += 4; // Destination IP
    pPacketPtr += IPheaderLength * 4 - 20; // Options

    IPtotalLength -= IPheaderLength * 4;
    return true;
}

// Helper: Parse UDP header
bool CAsterixPcapSubformat::parseUDPHeader(unsigned char *&pPacketPtr, bool bIPInvertByteOrder,
                                           unsigned short IPtotalLength, unsigned short &dataLength) {
    pPacketPtr += 2; // Source port
    pPacketPtr += 2; // Destination port

    dataLength = *reinterpret_cast<unsigned short *>(pPacketPtr);
    pPacketPtr += 2;

    if (bIPInvertByteOrder) {
        dataLength = convert_short(dataLength);
    }

    if (IPtotalLength != dataLength) {
        LOGERROR(1, "Wrong UDP data length");
        return false;
    }

    pPacketPtr += 2; // Checksum
    dataLength -= 8; // Subtract UDP header size
    return true;
}

// Helper: Parse ORADIS-wrapped ASTERIX data
void CAsterixPcapSubformat::parseOradisData(CAsterixFormatDescriptor &Descriptor,
                                            unsigned char *pPacketPtr, unsigned short dataLength,
                                            unsigned long nTimestamp) {
    while (dataLength > 0) {
        // Parse ORADIS header (6 bytes): ByteCount(2) + Time(4)
        unsigned short byteCount = pPacketPtr[0];
        byteCount = (byteCount << 8) | pPacketPtr[1];
        pPacketPtr += 6; // Skip byte count (2) + time (4)

        if (byteCount > dataLength) {
            break;
        }

        // Parse ASTERIX data
        AsterixData *tmpAsterixData = Descriptor.m_InputParser.parsePacket(pPacketPtr, byteCount - 6, nTimestamp);

        if (Descriptor.m_pAsterixData == nullptr) {
            Descriptor.m_pAsterixData = tmpAsterixData;
        } else {
            Descriptor.m_pAsterixData->m_lDataBlocks.splice(
                Descriptor.m_pAsterixData->m_lDataBlocks.end(),
                tmpAsterixData->m_lDataBlocks);
        }

        pPacketPtr += (byteCount - 6);
        dataLength -= byteCount;
    }
}

bool CAsterixPcapSubformat::ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                                       [[maybe_unused]] bool &discard, [[maybe_unused]] bool oradis) {
    auto &Descriptor = static_cast<CAsterixFormatDescriptor &>(formatDescriptor);
    static time_t lastFileTimeSec = 0;
    static useconds_t lastFileTimeUSec = 0;
    static time_t lastMyTimeSec = 0;
    static useconds_t lastMyTimeUSec = 0;

    // Read file header on first packet
    if (device.IsOnStart()) {
        lastFileTimeSec = 0;
        lastFileTimeUSec = 0;
        if (!readPcapFileHeader(Descriptor, device)) {
            return false;
        }
    }

    // Read PCAP packet header
    pcaprec_hdr_t pcapRecHeader;
    if (!device.Read(&pcapRecHeader, sizeof(pcapRecHeader))) {
        LOGERROR(1, "Couldn't read PCAP header.\n");
        return false;
    }

    // Calculate timestamp (milliseconds since midnight)
    unsigned long nTimestamp = (pcapRecHeader.ts_sec % 86400) * 1000 + pcapRecHeader.ts_usec / 1000;

    // Handle synchronous playback
    if (gSynchronous) {
        handleSynchronousDelay(pcapRecHeader, lastFileTimeSec, lastFileTimeUSec,
                               lastMyTimeSec, lastMyTimeUSec);
    }

    // Read PCAP packet data
    unsigned long nPacketBufferSize = pcapRecHeader.incl_len;
    if (Descriptor.m_bInvertByteOrder) {
        nPacketBufferSize = convert_long(nPacketBufferSize);
    }

    unsigned char *pPacketBuffer = Descriptor.GetNewBuffer(nPacketBufferSize);
    if (!device.Read(pPacketBuffer, nPacketBufferSize)) {
        LOGERROR(1, "Couldn't read PCAP packet.\n");
        return false;
    }

    // Parse network header
    bool bIPInvertByteOrder = false;
    unsigned char *pPacketPtr = parseNetworkHeader(pPacketBuffer, Descriptor, bIPInvertByteOrder);
    if (pPacketPtr == nullptr) {
        return false;
    }

    // Parse IP header
    unsigned short IPtotalLength = 0;
    if (!parseIPHeader(pPacketPtr, bIPInvertByteOrder, IPtotalLength)) {
        return false;
    }

    // Parse UDP header
    unsigned short dataLength = 0;
    if (!parseUDPHeader(pPacketPtr, bIPInvertByteOrder, IPtotalLength, dataLength)) {
        return false;
    }

    // Clean up old data
    if (Descriptor.m_pAsterixData) {
        delete Descriptor.m_pAsterixData;
        Descriptor.m_pAsterixData = nullptr;
    }

    // Parse ASTERIX data
    if (oradis) {
        parseOradisData(Descriptor, pPacketPtr, dataLength, nTimestamp);
    } else {
        Descriptor.m_pAsterixData = Descriptor.m_InputParser.parsePacket(pPacketPtr, dataLength, nTimestamp);
    }

    return true;
}

bool CAsterixPcapSubformat::WritePacket([[maybe_unused]] CBaseFormatDescriptor &formatDescriptor, [[maybe_unused]] CBaseDevice &device, [[maybe_unused]] bool &discard,
                                        [[maybe_unused]] bool oradis) {
    return false; //TODO
}

bool CAsterixPcapSubformat::ProcessPacket([[maybe_unused]] CBaseFormatDescriptor &formatDescriptor, [[maybe_unused]] CBaseDevice &device, [[maybe_unused]] bool &discard,
                                          [[maybe_unused]] bool oradis) {
    return true;
}

bool CAsterixPcapSubformat::Heartbeat([[maybe_unused]] CBaseFormatDescriptor &formatDescriptor, [[maybe_unused]] CBaseDevice &device, [[maybe_unused]] bool oradis) {
    return true;
}
