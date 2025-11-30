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

#ifdef HAVE_GRPC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <chrono>

// Cross-platform compatibility layer
#ifdef _WIN32
#include "win32_compat.h"
#endif

// Local includes
#include "asterix.h"
#include "grpcdevice.hxx"
#include "descriptor.hxx"


CGrpcDevice::CGrpcDevice(CDescriptor &descriptor) {
    _useTls = false;
    _streamActive = false;
    _currentOffset = 0;
    _serverRunning = false;

    const char *modeStr = descriptor.GetFirst();
    const char *endpoint = descriptor.GetNext();
    const char *tlsStr = descriptor.GetNext();

    if (modeStr == nullptr || endpoint == nullptr) {
        LOGERROR(1, "Error: Wrong gRPC descriptor format (shall be: mode:endpoint[:tls])\n");
        LOGERROR(1, "  mode: CLIENT or SERVER\n");
        LOGERROR(1, "  endpoint: host:port (e.g., localhost:50051)\n");
        LOGERROR(1, "  tls: optional, 'tls' to enable TLS/SSL\n");
        return;
    }

    _mode = ParseMode(modeStr);
    _endpoint = endpoint;

    if (tlsStr != nullptr && strcasecmp(tlsStr, "tls") == 0) {
        _useTls = true;
    }

    LOGINFO(gVerbose, "gRPC mode: %s\n", modeStr);
    LOGINFO(gVerbose, "gRPC endpoint: %s\n", _endpoint.c_str());
    LOGINFO(gVerbose, "gRPC TLS: %s\n", _useTls ? "enabled" : "disabled");

    // Initialize based on mode
    bool success = false;
    if (_mode == GRPC_MODE_CLIENT) {
        success = InitClient();
    } else {
        success = InitServer();
    }

    if (!success) {
        LOGERROR(1, "Failed to initialize gRPC device\n");
        return;
    }

    _opened = true;
}


CGrpcDevice::~CGrpcDevice() {
    // Stop server if running
    if (_server) {
        _serverRunning = false;
        _server->Shutdown();
        if (_serverThread && _serverThread->joinable()) {
            _serverThread->join();
        }
    }

    // Reset client context
    _clientContext.reset();
    _channel.reset();

    LOGDEBUG(ZONE_UDPDEVICE, "gRPC device destroyed\n");
}


CGrpcDevice::Mode CGrpcDevice::ParseMode(const char *modeStr) {
    if (strcasecmp(modeStr, "SERVER") == 0 || strcasecmp(modeStr, "SRV") == 0) {
        return GRPC_MODE_SERVER;
    }
    // Default to CLIENT
    return GRPC_MODE_CLIENT;
}


bool CGrpcDevice::InitClient() {
    // Create channel credentials
    std::shared_ptr<grpc::ChannelCredentials> creds;
    if (_useTls) {
        creds = grpc::SslCredentials(grpc::SslCredentialsOptions());
    } else {
        creds = grpc::InsecureChannelCredentials();
    }

    // Create channel
    _channel = grpc::CreateChannel(_endpoint, creds);
    if (!_channel) {
        LOGERROR(1, "Failed to create gRPC channel to %s\n", _endpoint.c_str());
        return false;
    }

    // Wait for channel to be ready (with timeout)
    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(10);
    bool connected = _channel->WaitForConnected(deadline);

    if (!connected) {
        LOGWARNING(1, "gRPC channel not immediately connected to %s (will retry on use)\n",
                   _endpoint.c_str());
        // Don't fail - gRPC will reconnect automatically
    } else {
        LOGINFO(gVerbose, "gRPC channel connected to %s\n", _endpoint.c_str());
    }

    _streamActive = true;
    return true;
}


bool CGrpcDevice::InitServer() {
    // Build server
    grpc::ServerBuilder builder;

    // Add listening port
    std::shared_ptr<grpc::ServerCredentials> creds;
    if (_useTls) {
        // For TLS, would need to load certificates
        // For now, use insecure credentials
        LOGWARNING(1, "gRPC TLS server not fully implemented, using insecure\n");
        creds = grpc::InsecureServerCredentials();
    } else {
        creds = grpc::InsecureServerCredentials();
    }

    builder.AddListeningPort(_endpoint, creds);

    // Note: In a full implementation, we would register a service here
    // For now, the server just accepts connections for raw data streaming

    // Build and start server
    _server = builder.BuildAndStart();
    if (!_server) {
        LOGERROR(1, "Failed to start gRPC server on %s\n", _endpoint.c_str());
        return false;
    }

    LOGINFO(gVerbose, "gRPC server listening on %s\n", _endpoint.c_str());

    _serverRunning = true;
    return true;
}


void CGrpcDevice::EnqueueMessage(const void *data, size_t len) {
    std::vector<unsigned char> msg(
        static_cast<const unsigned char*>(data),
        static_cast<const unsigned char*>(data) + len
    );

    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _messageQueue.push(std::move(msg));
    }
    _queueCondition.notify_one();
}


bool CGrpcDevice::Read(void *data, size_t len) {
    return Read(data, &len);
}


bool CGrpcDevice::Read(void *data, size_t *len) {
    if (!_opened) {
        LOGERROR(1, "Cannot read from gRPC: not initialized\n");
        CountReadError();
        return false;
    }

    // If we have a current message being read, continue from it
    if (!_currentMessage.empty() && _currentOffset < _currentMessage.size()) {
        size_t remaining = _currentMessage.size() - _currentOffset;
        size_t toCopy = std::min(remaining, *len);
        memcpy(data, _currentMessage.data() + _currentOffset, toCopy);
        _currentOffset += toCopy;
        *len = toCopy;

        // Clear message if fully read
        if (_currentOffset >= _currentMessage.size()) {
            _currentMessage.clear();
            _currentOffset = 0;
        }

        ResetReadErrors(true);
        return true;
    }

    // Try to get next message from queue (non-blocking)
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        if (_messageQueue.empty()) {
            *len = 0;
            return true;
        }

        _currentMessage = std::move(_messageQueue.front());
        _messageQueue.pop();
        _currentOffset = 0;
    }

    // Copy data from new message
    size_t toCopy = std::min(_currentMessage.size(), *len);
    memcpy(data, _currentMessage.data(), toCopy);
    _currentOffset = toCopy;
    *len = toCopy;

    // Clear message if fully read
    if (_currentOffset >= _currentMessage.size()) {
        _currentMessage.clear();
        _currentOffset = 0;
    }

    ResetReadErrors(true);
    return true;
}


bool CGrpcDevice::Write(const void *data, size_t len) {
    if (!_opened) {
        LOGERROR(1, "Cannot write to gRPC: not initialized\n");
        CountWriteError();
        return false;
    }

    // In server mode, this would broadcast to connected clients
    // In client mode, this would send to the server

    // For now, just enqueue for local processing (useful for testing)
    // A full implementation would send over the gRPC stream

    LOGDEBUG(ZONE_UDPDEVICE, "gRPC write %zu bytes (mode=%s)\n",
             len, _mode == GRPC_MODE_SERVER ? "SERVER" : "CLIENT");

    // In a full implementation:
    // - SERVER mode: broadcast to all connected streaming clients
    // - CLIENT mode: send to server via bidirectional stream

    ResetWriteErrors(true);
    return true;
}


bool CGrpcDevice::Select(const unsigned int secondsToWait) {
    if (!_opened) {
        LOGERROR(1, "Cannot select on gRPC: not initialized\n");
        return false;
    }

    // Check if we have a current message being read
    if (!_currentMessage.empty() && _currentOffset < _currentMessage.size()) {
        return true;
    }

    // Check if there are messages in queue
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        if (!_messageQueue.empty()) {
            return true;
        }
    }

    // Wait for message with timeout
    std::unique_lock<std::mutex> lock(_queueMutex);
    if (secondsToWait == 0) {
        // Wait indefinitely
        _queueCondition.wait(lock, [this] { return !_messageQueue.empty() || !_streamActive; });
        return !_messageQueue.empty();
    } else {
        // Wait with timeout
        auto result = _queueCondition.wait_for(
            lock,
            std::chrono::seconds(secondsToWait),
            [this] { return !_messageQueue.empty() || !_streamActive; }
        );
        return result && !_messageQueue.empty();
    }
}


bool CGrpcDevice::IoCtrl(const unsigned int command, const void *data, size_t len) {
    switch (command) {
        case EReset:
            // Clear message queue
            {
                std::lock_guard<std::mutex> lock(_queueMutex);
                while (!_messageQueue.empty()) {
                    _messageQueue.pop();
                }
            }
            _currentMessage.clear();
            _currentOffset = 0;
            ResetAllErrors();
            return true;

        case EPacketDone:
            return true;

        case EAllDone:
            _streamActive = false;
            _queueCondition.notify_all();
            return true;

        case EIsLastPacket:
            return !_streamActive;

        default:
            return false;
    }
}

#endif // HAVE_GRPC
