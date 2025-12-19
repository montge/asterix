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

#ifndef ASTERIXPCAPSUBFORMAT_HXX__
#define ASTERIXPCAPSUBFORMAT_HXX__

#include <ctime>
#ifdef _WIN32
typedef unsigned long useconds_t;
#endif

class CBaseDevice;

class CAsterixFormatDescriptor;

class InputParser;


/**
 * @class CAsterixPcapSubformat
 *
 * @brief Asterix pcap sub-format
 *
 * Specifies format of Asterix message.
 *
 */
class CAsterixPcapSubformat {
public:

    static bool
    ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard, bool oradis = false);

    static bool
    WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard, bool oradis = false);

    static bool
    ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard, bool oradis = false);

    static bool Heartbeat(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool oradis = false);

private:
    // PCAP header structures (must be declared before helper methods that use them)
    typedef struct pcap_hdr_s {
        unsigned int magic_number;   /* magic number */
        unsigned short version_major;  /* major version number */
        unsigned short version_minor;  /* minor version number */
        signed int thiszone;       /* GMT to local correction */
        unsigned int sigfigs;        /* accuracy of timestamps */
        unsigned int snaplen;        /* max length of captured packets, in octets */
        unsigned int network;        /* data link type */
    } pcap_hdr_t;

    typedef struct pcaprec_hdr_s {
        unsigned int ts_sec;         /* timestamp seconds */
        unsigned int ts_usec;        /* timestamp microseconds */
        unsigned int incl_len;       /* number of octets of packet saved in file */
        unsigned int orig_len;       /* actual length of packet */
    } pcaprec_hdr_t;

    // Helper methods to reduce cognitive complexity of ReadPacket
    static bool readPcapFileHeader(CAsterixFormatDescriptor &Descriptor, CBaseDevice &device);
    static void handleSynchronousDelay(const pcaprec_hdr_t &recHeader,
                                       time_t &lastFileTimeSec, useconds_t &lastFileTimeUSec,
                                       time_t &lastMyTimeSec, useconds_t &lastMyTimeUSec);
    static unsigned char* parseNetworkHeader(unsigned char *pPacketBuffer,
                                             CAsterixFormatDescriptor &Descriptor,
                                             bool &bIPInvertByteOrder);
    static bool parseIPHeader(unsigned char *&pPacketPtr, bool bIPInvertByteOrder,
                              unsigned short &IPtotalLength);
    static bool parseUDPHeader(unsigned char *&pPacketPtr, bool bIPInvertByteOrder,
                               unsigned short IPtotalLength, unsigned short &dataLength);
    static void parseOradisData(CAsterixFormatDescriptor &Descriptor,
                                unsigned char *pPacketPtr, unsigned short dataLength,
                                unsigned long nTimestamp);

    static short convert_short(short in) {
        short out;
        auto* p_in = reinterpret_cast<char*>(&in);
        auto* p_out = reinterpret_cast<char*>(&out);
        p_out[0] = p_in[1];
        p_out[1] = p_in[0];
        return out;
    }

    static long convert_long(long in) {
        long out;
        auto* p_in = reinterpret_cast<char*>(&in);
        auto* p_out = reinterpret_cast<char*>(&out);
        p_out[0] = p_in[3];
        p_out[1] = p_in[2];
        p_out[2] = p_in[1];
        p_out[3] = p_in[0];
        return out;
    }
};

#endif
