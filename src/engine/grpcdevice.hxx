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
 * AUTHORS: Claude Code (ASTERIX gRPC integration)
 *
 */

#ifndef GRPCDEVICE_HXX__
#define GRPCDEVICE_HXX__

#ifdef HAVE_GRPC

#include <grpcpp/grpcpp.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>
#include <atomic>

#include "basedevice.hxx"
#include "descriptor.hxx"

#define MAX_GRPC_MESSAGE_SIZE    65536

/**
 * @class CGrpcDevice
 *
 * @brief gRPC device for high-performance RPC-based ASTERIX transport.
 *
 * Supports streaming raw ASTERIX data over gRPC using a simple bytes-based
 * protocol. Can operate as a client (connecting to a gRPC server) or as
 * a server (accepting connections from gRPC clients).
 *
 * Descriptor format: mode:endpoint[:tls]
 *   - mode: CLIENT or SERVER
 *   - endpoint: host:port (e.g., localhost:50051)
 *   - tls: Optional, "tls" to enable TLS/SSL
 *
 * Examples:
 *   CLIENT:localhost:50051           - Connect to local gRPC server
 *   CLIENT:radar.example.com:50051:tls - Connect with TLS
 *   SERVER:0.0.0.0:50051             - Start gRPC server on all interfaces
 *
 * The gRPC service uses a simple streaming protocol:
 *   - Server streams AsterixData messages containing raw ASTERIX bytes
 *   - Client receives and processes each message
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 *        <CDescriptor>
 */
class CGrpcDevice : public CBaseDevice {
public:
    /**
     * Mode enumeration
     */
    enum Mode {
        GRPC_MODE_CLIENT,   // Client connecting to server
        GRPC_MODE_SERVER    // Server accepting connections
    };

private:
    Mode _mode;                          // CLIENT or SERVER mode
    std::string _endpoint;               // host:port endpoint
    bool _useTls;                        // Enable TLS/SSL

    // gRPC client components
    std::shared_ptr<grpc::Channel> _channel;
    std::unique_ptr<grpc::ClientContext> _clientContext;

    // gRPC server components
    std::unique_ptr<grpc::Server> _server;
    std::unique_ptr<std::thread> _serverThread;
    std::atomic<bool> _serverRunning;

    // Message queue for received data
    std::queue<std::vector<unsigned char>> _messageQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;

    // Current message being read
    std::vector<unsigned char> _currentMessage;
    size_t _currentOffset;

    // Streaming state
    bool _streamActive;

private:
    /**
     * Initialize gRPC client
     */
    bool InitClient();

    /**
     * Initialize gRPC server
     */
    bool InitServer();

    /**
     * Parse mode string to enum
     */
    Mode ParseMode(const char *modeStr);

    /**
     * Add message to queue (thread-safe)
     */
    void EnqueueMessage(const void *data, size_t len);

public:

    /**
     * Class constructor which uses descriptor
     *
     * @param descriptor Descriptor with format: mode:endpoint[:tls]
     */
    CGrpcDevice(CDescriptor &descriptor);

    /**
     * Class destructor.
     */
    virtual ~CGrpcDevice();

    /**
     * Read data from gRPC stream (exact length)
     *
     * @param data Buffer to store received data
     * @param len Number of bytes to read
     * @return true if successful, false on error
     */
    virtual bool Read(void *data, size_t len);

    /**
     * Read data from gRPC stream (variable length)
     *
     * @param data Buffer to store received data
     * @param len Input: max buffer size, Output: actual bytes read
     * @return true if successful, false on error
     */
    virtual bool Read(void *data, size_t *len);

    /**
     * Write data to gRPC stream (send to clients)
     *
     * @param data Data to send
     * @param len Number of bytes to send
     * @return true if successful, false on error
     */
    virtual bool Write(const void *data, size_t len);

    /**
     * Wait for data availability
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
     * Returns true - gRPC messages are packet-oriented
     */
    virtual bool IsPacketDevice() { return true; }

    /**
     * Returns maximum message size supported
     */
    virtual unsigned int MaxPacketSize() { return MAX_GRPC_MESSAGE_SIZE; }

    /**
     * Returns 0 - gRPC doesn't support "bytes left to read" concept
     */
    virtual unsigned int BytesLeftToRead() { return 0; }
};

#endif // HAVE_GRPC

#endif // GRPCDEVICE_HXX__
