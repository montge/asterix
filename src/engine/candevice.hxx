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
 */
#ifndef CANDEVICE_HXX__
#define CANDEVICE_HXX__

#ifdef HAVE_SOCKETCAN
#ifndef _WIN32

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>

#include <cstdint>
#include <map>
#include <vector>
#include <chrono>
#include <string>

#include "basedevice.hxx"
#include "descriptor.hxx"

// Payload sizes (frame size minus 1 header byte)
static const size_t CAN_PAYLOAD_SIZE = 7;
static const size_t CANFD_PAYLOAD_SIZE = 63;

// Fragment header bit flags
static const uint8_t FRAGMENT_LAST_FLAG = 0x80;
static const uint8_t FRAGMENT_INDEX_MASK = 0x7F;

/**
 * @brief CAN frame type: classic (8-byte) or FD (64-byte)
 */
enum CanFrameType {
    CAN_TYPE_CLASSIC = 0,
    CAN_TYPE_FD = 1
};

/**
 * @brief State for reassembling fragmented CAN frames into complete messages
 */
struct ReassemblyState {
    uint8_t category;
    std::map<uint8_t, std::vector<uint8_t>> fragments; // index -> payload data
    std::chrono::steady_clock::time_point lastUpdate;

    ReassemblyState() : category(0), lastUpdate(std::chrono::steady_clock::now()) {}

    explicit ReassemblyState(uint8_t cat)
        : category(cat), lastUpdate(std::chrono::steady_clock::now()) {}

    bool IsExpired(unsigned int timeoutMs) const;
    bool IsComplete(uint8_t lastIndex) const;
    std::vector<uint8_t> Assemble(uint8_t lastIndex) const;
};

/**
 * @class CCanDevice
 *
 * @brief SocketCAN device for receiving ASTERIX data over CAN bus.
 *
 * Supports classic CAN and CAN FD frames with automatic fragment
 * reassembly. Linux-only (requires kernel SocketCAN support).
 *
 * Descriptor format: interface[:fd[:timeout_ms]]
 *   - interface: CAN interface name (e.g., "can0", "vcan0")
 *   - fd: "fd" for CAN FD, "classic" or omit for classic CAN
 *   - timeout_ms: reassembly timeout in milliseconds (default 1000)
 *
 * @see CBaseDevice, CDeviceFactory
 */
class CCanDevice : public CBaseDevice {
private:
    int _socketDesc;
    std::string _interface;
    CanFrameType _frameType;
    unsigned int _reassemblyTimeoutMs;

    std::map<uint8_t, ReassemblyState> _reassemblyStates;
    std::vector<uint8_t> _outputBuffer;

public:
    explicit CCanDevice(CDescriptor &descriptor);
    ~CCanDevice() override;

    bool Read(void *data, size_t len) override;
    bool Read(void *data, size_t *len) override;
    bool Write(const void *data, size_t len) override;
    bool Select(const unsigned int secondsToWait) override;
    bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0) override;
    bool IsPacketDevice() override { return true; }
    unsigned int MaxPacketSize() override { return 65536; }

    /**
     * @brief Build an 11-bit CAN ID from category and fragment index
     *
     * CAN ID format: bits [10:8] = (category >> 5) & 0x07, bits [7:0] = fragment_index
     */
    static uint32_t BuildCanId(uint8_t category, uint8_t fragmentIndex);

    /**
     * @brief Parse CAN ID to extract category (high bits) and fragment sequence
     */
    static void ParseCanId(uint32_t canId, uint8_t &catHigh, uint8_t &fragSeq);

    /**
     * @brief Build a fragment header byte
     *
     * bit 7 = is_last, bits [6:0] = fragment_index
     */
    static uint8_t BuildFragmentHeader(uint8_t fragmentIndex, bool isLast);

    /**
     * @brief Parse a fragment header byte
     */
    static void ParseFragmentHeader(uint8_t header, uint8_t &fragmentIndex, bool &isLast);

private:
    void Init(const char *interface, bool useFd);
    void CleanupExpiredStates();
    size_t PayloadSize() const;
};

#endif // _WIN32
#endif // HAVE_SOCKETCAN

#endif // CANDEVICE_HXX__
