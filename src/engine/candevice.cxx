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

// SocketCAN device - Linux only
#ifdef HAVE_SOCKETCAN
#ifndef _WIN32

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "asterix.h"
#include "candevice.hxx"


// --- ReassemblyState ---

bool ReassemblyState::IsExpired(unsigned int timeoutMs) const {
    auto elapsed = std::chrono::steady_clock::now() - lastUpdate;
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    return elapsedMs > static_cast<long long>(timeoutMs);
}

bool ReassemblyState::IsComplete(uint8_t lastIndex) const {
    for (uint8_t i = 0; i <= lastIndex; i++) {
        if (fragments.find(i) == fragments.end()) {
            return false;
        }
    }
    return true;
}

std::vector<uint8_t> ReassemblyState::Assemble(uint8_t lastIndex) const {
    std::vector<uint8_t> result;
    for (uint8_t i = 0; i <= lastIndex; i++) {
        auto it = fragments.find(i);
        if (it == fragments.end()) {
            return std::vector<uint8_t>(); // missing fragment
        }
        result.insert(result.end(), it->second.begin(), it->second.end());
    }
    return result;
}


// --- CCanDevice static helpers ---

uint32_t CCanDevice::BuildCanId(uint8_t category, uint8_t fragmentIndex) {
    uint32_t catHigh = (static_cast<uint32_t>(category) >> 5) & 0x07;
    uint32_t frag = static_cast<uint32_t>(fragmentIndex);
    return (catHigh << 8) | frag;
}

void CCanDevice::ParseCanId(uint32_t canId, uint8_t &catHigh, uint8_t &fragSeq) {
    catHigh = static_cast<uint8_t>(((canId >> 8) & 0x07) << 5);
    fragSeq = static_cast<uint8_t>(canId & 0xFF);
}

uint8_t CCanDevice::BuildFragmentHeader(uint8_t fragmentIndex, bool isLast) {
    uint8_t header = fragmentIndex & FRAGMENT_INDEX_MASK;
    if (isLast) {
        header |= FRAGMENT_LAST_FLAG;
    }
    return header;
}

void CCanDevice::ParseFragmentHeader(uint8_t header, uint8_t &fragmentIndex, bool &isLast) {
    fragmentIndex = header & FRAGMENT_INDEX_MASK;
    isLast = (header & FRAGMENT_LAST_FLAG) != 0;
}


// --- CCanDevice ---

CCanDevice::CCanDevice(CDescriptor &descriptor)
    : _socketDesc(-1), _frameType(CAN_TYPE_CLASSIC), _reassemblyTimeoutMs(1000) {
    const char *iface = descriptor.GetFirst();
    const char *mode = descriptor.GetNext();
    const char *timeout = descriptor.GetNext();

    _interface = iface ? iface : "can0";

    if (mode != nullptr && strcasecmp(mode, "fd") == 0) {
        _frameType = CAN_TYPE_FD;
    }

    if (timeout != nullptr) {
        int val = atoi(timeout);
        if (val > 0) {
            _reassemblyTimeoutMs = static_cast<unsigned int>(val);
        }
    }

    Init(_interface.c_str(), _frameType == CAN_TYPE_FD);
}

CCanDevice::~CCanDevice() {
    if (_socketDesc >= 0) {
        close(_socketDesc);
        _socketDesc = -1;
    }
}

void CCanDevice::Init(const char *interface, bool useFd) {
    _opened = false;
    _socketDesc = -1;
    CBaseDevice::ResetAllErrors();

    if (interface == nullptr) {
        LOGERROR(1, "Cannot open CAN device: null interface name\n");
        return;
    }

    // Create SocketCAN socket
    _socketDesc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (_socketDesc < 0) {
        LOGERROR(1, "Cannot create CAN socket: %s\n", strerror(errno));
        return;
    }

    // Resolve interface index
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(_socketDesc, SIOCGIFINDEX, &ifr) < 0) {
        LOGERROR(1, "Cannot find CAN interface '%s': %s\n", interface, strerror(errno));
        close(_socketDesc);
        _socketDesc = -1;
        return;
    }

    // Bind to interface
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(_socketDesc, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
        LOGERROR(1, "Cannot bind to CAN interface '%s': %s\n", interface, strerror(errno));
        close(_socketDesc);
        _socketDesc = -1;
        return;
    }

    if (useFd) {
        LOGINFO(gVerbose, "CAN FD device opened on interface %s\n", interface);
    } else {
        LOGINFO(gVerbose, "CAN device opened on interface %s\n", interface);
    }

    _opened = true;
}

size_t CCanDevice::PayloadSize() const {
    return (_frameType == CAN_TYPE_FD) ? CANFD_PAYLOAD_SIZE : CAN_PAYLOAD_SIZE;
}

bool CCanDevice::Read(void *data, size_t len) {
    return Read(data, &len);
}

bool CCanDevice::Read(void *data, size_t *len) {
    if (_socketDesc < 0 || !_opened) {
        CountReadError();
        *len = 0;
        return false;
    }

    // Clean up expired reassembly states
    CleanupExpiredStates();

    // If we have a buffered complete message, return it
    if (!_outputBuffer.empty()) {
        size_t copyLen = _outputBuffer.size();
        if (copyLen > *len) {
            copyLen = *len;
        }
        memcpy(data, _outputBuffer.data(), copyLen);
        *len = copyLen;
        _outputBuffer.clear();
        ResetReadErrors(true);
        return true;
    }

    // Read CAN frames and reassemble
    struct can_frame frame;
    ssize_t nbytes = read(_socketDesc, &frame, sizeof(frame));

    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            *len = 0;
            return false;
        }
        LOGERROR(1, "Error reading from CAN socket: %s\n", strerror(errno));
        CountReadError();
        *len = 0;
        return false;
    }

    if (nbytes < static_cast<ssize_t>(sizeof(struct can_frame))) {
        CountReadError();
        *len = 0;
        return false;
    }

    // Mask to get 11-bit standard ID
    uint32_t canId = frame.can_id & CAN_SFF_MASK;

    // Need at least 1 byte for fragment header
    if (frame.can_dlc < 1) {
        *len = 0;
        return false;
    }

    // Parse CAN ID
    uint8_t catHigh = 0;
    uint8_t fragSeq = 0;
    ParseCanId(canId, catHigh, fragSeq);

    // Parse fragment header
    uint8_t fragmentIndex = 0;
    bool isLast = false;
    ParseFragmentHeader(frame.data[0], fragmentIndex, isLast);

    // Extract payload (after header byte)
    size_t payloadLen = frame.can_dlc - 1;
    std::vector<uint8_t> payload(frame.data + 1, frame.data + 1 + payloadLen);

    // Category key uses the high bits recovered from the CAN ID
    // (known limitation: categories sharing the same high 3 bits can collide)
    uint8_t category = catHigh;

    // Get or create reassembly state
    auto it = _reassemblyStates.find(category);
    if (it == _reassemblyStates.end()) {
        _reassemblyStates[category] = ReassemblyState(category);
        it = _reassemblyStates.find(category);
    }

    ReassemblyState &state = it->second;
    state.lastUpdate = std::chrono::steady_clock::now();
    state.fragments[fragmentIndex] = std::move(payload);

    // If this is the last fragment, try to reassemble
    if (isLast) {
        if (state.IsComplete(fragmentIndex)) {
            std::vector<uint8_t> assembled = state.Assemble(fragmentIndex);
            _reassemblyStates.erase(category);

            if (!assembled.empty()) {
                size_t copyLen = assembled.size();
                if (copyLen > *len) {
                    // Store excess in output buffer for next Read
                    _outputBuffer = std::move(assembled);
                    copyLen = *len;
                    memcpy(data, _outputBuffer.data(), copyLen);
                    _outputBuffer.erase(_outputBuffer.begin(),
                                        _outputBuffer.begin() + static_cast<long>(copyLen));
                } else {
                    memcpy(data, assembled.data(), copyLen);
                }
                *len = copyLen;
                ResetReadErrors(true);
                return true;
            }
        } else {
            // Missing fragments - discard
            _reassemblyStates.erase(category);
        }
    }

    // Not yet complete, need more frames
    *len = 0;
    return false;
}

bool CCanDevice::Write([[maybe_unused]] const void *data, [[maybe_unused]] size_t len) {
    // CAN output not implemented - receive-only device
    return false;
}

bool CCanDevice::Select(const unsigned int secondsToWait) {
    if (_socketDesc < 0 || !_opened) {
        return false;
    }

    // If we have buffered data, it's ready immediately
    if (!_outputBuffer.empty()) {
        return true;
    }

    fd_set descToRead;
    FD_ZERO(&descToRead);
    FD_SET(_socketDesc, &descToRead);

    int selectVal;
    if (secondsToWait) {
        struct timeval timeout;
        timeout.tv_sec = secondsToWait;
        timeout.tv_usec = 0;
        selectVal = select(_socketDesc + 1, &descToRead, nullptr, nullptr, &timeout);
    } else {
        selectVal = select(_socketDesc + 1, &descToRead, nullptr, nullptr, nullptr);
    }

    return (selectVal > 0);
}

bool CCanDevice::IoCtrl(const unsigned int command, [[maybe_unused]] const void *data, [[maybe_unused]] size_t len) {
    switch (command) {
        case EReset:
            _reassemblyStates.clear();
            _outputBuffer.clear();
            return true;
        case EAllDone:
            return true;
        case EPacketDone:
            return false;
        default:
            return false;
    }
}

void CCanDevice::CleanupExpiredStates() {
    auto it = _reassemblyStates.begin();
    while (it != _reassemblyStates.end()) {
        if (it->second.IsExpired(_reassemblyTimeoutMs)) {
            it = _reassemblyStates.erase(it);
        } else {
            ++it;
        }
    }
}

#endif // _WIN32
#endif // HAVE_SOCKETCAN
