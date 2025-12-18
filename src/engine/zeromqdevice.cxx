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
 * AUTHORS: Claude Code (ASTERIX ZeroMQ integration)
 *
 */

#ifdef HAVE_ZEROMQ

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>

// Cross-platform compatibility layer
#ifdef _WIN32
#include "win32_compat.h"
#endif

// Local includes
#include "asterix.h"
#include "zeromqdevice.hxx"
#include "descriptor.hxx"


CZeromqDevice::CZeromqDevice(CDescriptor &descriptor) {
    _context = nullptr;
    _socket = nullptr;
    _isBound = false;
    _topic = "";

    const char *typeStr = descriptor.GetFirst();
    const char *endpoint = descriptor.GetNext();
    const char *bindOption = descriptor.GetNext();

    if (typeStr == nullptr || endpoint == nullptr) {
        LOGERROR(1, "Error: Wrong ZeroMQ descriptor format (shall be: type:endpoint[:bind])\n");
        LOGERROR(1, "  type: SUB, PULL, PUB, or PUSH\n");
        LOGERROR(1, "  endpoint: tcp://host:port, ipc:///path, or inproc://name\n");
        LOGERROR(1, "  bind: optional, 'bind' to bind instead of connect\n");
        return;
    }

    bool shouldBind = false;
    if (bindOption != nullptr && strcasecmp(bindOption, "bind") == 0) {
        shouldBind = true;
    }

    LOGINFO(gVerbose, "ZeroMQ socket type: %s\n", typeStr);
    LOGINFO(gVerbose, "ZeroMQ endpoint: %s\n", endpoint);
    LOGINFO(gVerbose, "ZeroMQ bind: %s\n", shouldBind ? "yes" : "no (connect)");

    // Initialize the socket
    if (!InitSocket(typeStr, endpoint, shouldBind)) {
        LOGERROR(1, "Failed to initialize ZeroMQ socket\n");
        return;
    }

    _opened = true;
}


CZeromqDevice::~CZeromqDevice() {
    if (_socket != nullptr) {
        zmq_close(_socket);
        _socket = nullptr;
    }

    if (_context != nullptr) {
        zmq_ctx_destroy(_context);
        _context = nullptr;
    }

    LOGDEBUG(ZONE_UDPDEVICE, "ZeroMQ device destroyed\n");
}


CZeromqDevice::SocketType CZeromqDevice::ParseSocketType(const char *typeStr) {
    if (strcasecmp(typeStr, "SUB") == 0) {
        return ZMQ_TYPE_SUB;
    } else if (strcasecmp(typeStr, "PULL") == 0) {
        return ZMQ_TYPE_PULL;
    } else if (strcasecmp(typeStr, "PUB") == 0) {
        return ZMQ_TYPE_PUB;
    } else if (strcasecmp(typeStr, "PUSH") == 0) {
        return ZMQ_TYPE_PUSH;
    }

    // Default to SUB
    LOGWARNING(1, "Unknown ZeroMQ socket type '%s', defaulting to SUB\n", typeStr);
    return ZMQ_TYPE_SUB;
}


bool CZeromqDevice::InitSocket(const char *typeStr, const char *endpoint, bool bind) {
    // Parse socket type
    _socketType = ParseSocketType(typeStr);
    _endpoint = endpoint;
    _isBound = bind;

    // Create ZeroMQ context
    _context = zmq_ctx_new();
    if (_context == nullptr) {
        LOGERROR(1, "Failed to create ZeroMQ context: %s\n", zmq_strerror(errno));
        return false;
    }

    // Map our socket type to ZeroMQ socket type
    int zmqSocketType;
    switch (_socketType) {
        case ZMQ_TYPE_SUB:
            zmqSocketType = ZMQ_SUB;
            break;
        case ZMQ_TYPE_PULL:
            zmqSocketType = ZMQ_PULL;
            break;
        case ZMQ_TYPE_PUB:
            zmqSocketType = ZMQ_PUB;
            break;
        case ZMQ_TYPE_PUSH:
            zmqSocketType = ZMQ_PUSH;
            break;
        default:
            zmqSocketType = ZMQ_SUB;
            break;
    }

    // Create socket
    _socket = zmq_socket(_context, zmqSocketType);
    if (_socket == nullptr) {
        LOGERROR(1, "Failed to create ZeroMQ socket: %s\n", zmq_strerror(errno));
        zmq_ctx_destroy(_context);
        _context = nullptr;
        return false;
    }

    // Set socket options
    int linger = 0;  // Don't wait on close
    zmq_setsockopt(_socket, ZMQ_LINGER, &linger, sizeof(linger));

    // Set receive timeout (1 second) for non-blocking behavior
    int rcvtimeo = 1000;  // milliseconds
    zmq_setsockopt(_socket, ZMQ_RCVTIMEO, &rcvtimeo, sizeof(rcvtimeo));

    // Set high water mark for receive buffer
    int rcvhwm = 10000;  // messages
    zmq_setsockopt(_socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));

    // For SUB socket, subscribe to all messages by default
    if (_socketType == ZMQ_TYPE_SUB) {
        zmq_setsockopt(_socket, ZMQ_SUBSCRIBE, "", 0);
    }

    // Bind or connect
    int rc;
    if (bind) {
        rc = zmq_bind(_socket, endpoint);
        if (rc != 0) {
            LOGERROR(1, "Failed to bind ZeroMQ socket to %s: %s\n",
                     endpoint, zmq_strerror(errno));
            zmq_close(_socket);
            _socket = nullptr;
            zmq_ctx_destroy(_context);
            _context = nullptr;
            return false;
        }
        LOGINFO(gVerbose, "ZeroMQ socket bound to %s\n", endpoint);
    } else {
        rc = zmq_connect(_socket, endpoint);
        if (rc != 0) {
            LOGERROR(1, "Failed to connect ZeroMQ socket to %s: %s\n",
                     endpoint, zmq_strerror(errno));
            zmq_close(_socket);
            _socket = nullptr;
            zmq_ctx_destroy(_context);
            _context = nullptr;
            return false;
        }
        LOGINFO(gVerbose, "ZeroMQ socket connected to %s\n", endpoint);
    }

    // Setup poll item for Select()
    _pollItem.socket = _socket;
    _pollItem.fd = 0;
    _pollItem.events = ZMQ_POLLIN;
    _pollItem.revents = 0;

    return true;
}


bool CZeromqDevice::Read(void *data, size_t len) {
    return Read(data, &len);
}


bool CZeromqDevice::Read(void *data, size_t *len) {
    if (!_opened || _socket == nullptr) {
        LOGERROR(1, "Cannot read from ZeroMQ: socket not initialized\n");
        CountReadError();
        return false;
    }

    // For PUB/PUSH sockets, read is not supported
    if (_socketType == ZMQ_TYPE_PUB || _socketType == ZMQ_TYPE_PUSH) {
        LOGERROR(1, "Cannot read from ZeroMQ PUB/PUSH socket\n");
        CountReadError();
        return false;
    }

    // Receive message with ZMQ_DONTWAIT for non-blocking
    int nbytes = zmq_recv(_socket, data, *len, ZMQ_DONTWAIT);

    if (nbytes < 0) {
        if (errno == EAGAIN) {
            // No message available (non-blocking)
            *len = 0;
            return true;
        }
        LOGERROR(1, "ZeroMQ recv error: %s\n", zmq_strerror(errno));
        CountReadError();
        return false;
    }

    *len = static_cast<size_t>(nbytes);

    LOGDEBUG(ZONE_UDPDEVICE, "ZeroMQ received %d bytes from %s\n",
             nbytes, _endpoint.c_str());

    ResetReadErrors(true);
    return true;
}


bool CZeromqDevice::Write(const void *data, size_t len) {
    if (!_opened || _socket == nullptr) {
        LOGERROR(1, "Cannot write to ZeroMQ: socket not initialized\n");
        CountWriteError();
        return false;
    }

    // For SUB/PULL sockets, write is not supported
    if (_socketType == ZMQ_TYPE_SUB || _socketType == ZMQ_TYPE_PULL) {
        LOGERROR(1, "Cannot write to ZeroMQ SUB/PULL socket\n");
        CountWriteError();
        return false;
    }

    // Send message
    int nbytes = zmq_send(_socket, data, len, 0);

    if (nbytes < 0) {
        LOGERROR(1, "ZeroMQ send error: %s\n", zmq_strerror(errno));
        CountWriteError();
        return false;
    }

    LOGDEBUG(ZONE_UDPDEVICE, "ZeroMQ sent %d bytes to %s\n",
             nbytes, _endpoint.c_str());

    ResetWriteErrors(true);
    return true;
}


/**
 * @brief Waits for data availability on ZeroMQ socket using zmq_poll
 *
 * Uses ZeroMQ's native polling mechanism for efficient waiting.
 *
 * @param secondsToWait Timeout in seconds (0 = wait indefinitely)
 * @return true if data available, false on timeout or error
 */
bool CZeromqDevice::Select(const unsigned int secondsToWait) {
    if (!_opened || _socket == nullptr) {
        LOGERROR(1, "Cannot select on ZeroMQ: socket not initialized\n");
        return false;
    }

    // Convert seconds to milliseconds for zmq_poll
    // 0 means infinite timeout in our API, but zmq_poll uses -1 for infinite
    long timeout_ms = (secondsToWait == 0) ? -1 : static_cast<long>(secondsToWait * 1000);

    // Reset poll events
    _pollItem.revents = 0;

    int rc = zmq_poll(&_pollItem, 1, timeout_ms);

    if (rc < 0) {
        LOGERROR(1, "ZeroMQ poll error: %s\n", zmq_strerror(errno));
        return false;
    }

    if (rc == 0) {
        // Timeout
        return false;
    }

    // Check if data is available
    return (_pollItem.revents & ZMQ_POLLIN) != 0;
}


bool CZeromqDevice::IoCtrl(const unsigned int command, const void *data, size_t len) {
    switch (command) {
        case EReset:
            // Reset any internal state if needed
            ResetAllErrors();
            return true;

        case EPacketDone:
            // Packet processing complete - no action needed for ZeroMQ
            return true;

        case EAllDone:
            // All processing complete - could disconnect here
            return true;

        case EIsLastPacket:
            // ZeroMQ doesn't have a concept of "last packet"
            return false;

        default:
            return false;
    }
}


void CZeromqDevice::SetTopic(const std::string &topic) {
    if (_socketType != ZMQ_TYPE_SUB) {
        LOGWARNING(1, "SetTopic only applicable to SUB sockets\n");
        return;
    }

    if (_socket == nullptr) {
        _topic = topic;
        return;
    }

    // Unsubscribe from previous topic
    if (!_topic.empty()) {
        zmq_setsockopt(_socket, ZMQ_UNSUBSCRIBE, _topic.c_str(), _topic.length());
    } else {
        // Unsubscribe from "all messages"
        zmq_setsockopt(_socket, ZMQ_UNSUBSCRIBE, "", 0);
    }

    // Subscribe to new topic
    _topic = topic;
    zmq_setsockopt(_socket, ZMQ_SUBSCRIBE, _topic.c_str(), _topic.length());

    LOGINFO(gVerbose, "ZeroMQ SUB topic set to '%s'\n",
            _topic.empty() ? "(all)" : _topic.c_str());
}

#endif // HAVE_ZEROMQ
