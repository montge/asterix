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

#ifndef ZEROMQDEVICE_HXX__
#define ZEROMQDEVICE_HXX__

#ifdef HAVE_ZEROMQ

#include <zmq.h>
#include <string>

#include "basedevice.hxx"
#include "descriptor.hxx"

#define MAX_ZMQ_MESSAGE_SIZE    65536

/**
 * @class CZeromqDevice
 *
 * @brief ZeroMQ device for low-latency messaging transport.
 *
 * Supports SUB (subscriber) and PULL socket types for receiving ASTERIX data.
 * The device can connect to remote endpoints or bind to local addresses.
 *
 * Descriptor format: type:endpoint[:bind]
 *   - type: SUB or PULL
 *   - endpoint: ZeroMQ endpoint (tcp://host:port, ipc:///path, inproc://name)
 *   - bind: Optional, "bind" to bind instead of connect (default: connect)
 *
 * Examples:
 *   SUB:tcp://192.168.1.10:5555         - Subscribe to remote publisher
 *   PULL:tcp://*:5556:bind              - Bind PULL socket on port 5556
 *   SUB:ipc:///tmp/asterix.ipc          - Subscribe via IPC
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 *        <CDescriptor>
 */
class CZeromqDevice : public CBaseDevice {
public:
    /**
     * Socket type enumeration
     */
    enum SocketType {
        ZMQ_TYPE_SUB,   // Subscriber socket
        ZMQ_TYPE_PULL,  // Pull socket (pipeline pattern)
        ZMQ_TYPE_PUB,   // Publisher socket (for write)
        ZMQ_TYPE_PUSH   // Push socket (for write)
    };

private:
    void *_context;           // ZeroMQ context
    void *_socket;            // ZeroMQ socket
    SocketType _socketType;   // Socket type (SUB, PULL, etc.)
    std::string _endpoint;    // Endpoint string
    bool _isBound;            // True if bound, false if connected
    zmq_pollitem_t _pollItem; // Poll item for Select()

    // For topic filtering (SUB socket only)
    std::string _topic;

private:
    /**
     * Initialize the ZeroMQ context and socket
     */
    bool InitSocket(const char *typeStr, const char *endpoint, bool bind);

    /**
     * Parse socket type string to enum
     */
    SocketType ParseSocketType(const char *typeStr);

public:

    /**
     * Class constructor which uses descriptor
     *
     * @param descriptor Descriptor with format: type:endpoint[:bind]
     */
    CZeromqDevice(CDescriptor &descriptor);

    /**
     * Class destructor.
     */
    virtual ~CZeromqDevice();

    /**
     * Read data from ZeroMQ socket (exact length)
     *
     * @param data Buffer to store received data
     * @param len Number of bytes to read
     * @return true if successful, false on error
     */
    virtual bool Read(void *data, size_t len);

    /**
     * Read data from ZeroMQ socket (variable length)
     *
     * @param data Buffer to store received data
     * @param len Input: max buffer size, Output: actual bytes read
     * @return true if successful, false on error
     */
    virtual bool Read(void *data, size_t *len);

    /**
     * Write data to ZeroMQ socket
     *
     * @param data Data to send
     * @param len Number of bytes to send
     * @return true if successful, false on error
     */
    virtual bool Write(const void *data, size_t len);

    /**
     * Wait for data availability using zmq_poll
     *
     * @param secondsToWait Timeout in seconds (0 = wait indefinitely)
     * @return true if data available, false on timeout or error
     */
    virtual bool Select(const unsigned int secondsToWait);

    /**
     * IO control operations
     *
     * @param command Control command (EReset, EPacketDone, etc.)
     * @param data Optional data parameter
     * @param len Optional length parameter
     * @return true if successful, false otherwise
     */
    virtual bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0);

    /**
     * Returns true - ZeroMQ is message-oriented (packet device)
     */
    virtual bool IsPacketDevice() { return true; }

    /**
     * Returns maximum message size supported
     */
    virtual unsigned int MaxPacketSize() { return MAX_ZMQ_MESSAGE_SIZE; }

    /**
     * Returns 0 - ZeroMQ doesn't support "bytes left to read" concept
     */
    virtual unsigned int BytesLeftToRead() { return 0; }

    /**
     * Set topic filter for SUB socket
     *
     * @param topic Topic prefix to subscribe to (empty = all messages)
     */
    void SetTopic(const std::string &topic);
};

#endif // HAVE_ZEROMQ

#endif // ZEROMQDEVICE_HXX__
