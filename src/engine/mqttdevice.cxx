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
 * AUTHORS: Claude Code (ASTERIX MQTT integration)
 *
 */

#ifdef HAVE_MQTT

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
#include "mqttdevice.hxx"
#include "descriptor.hxx"


// Static callback wrappers
void CMqttDevice::OnMessageCallback(struct mosquitto *mosq, void *userdata,
                                    const struct mosquitto_message *message) {
    CMqttDevice *device = static_cast<CMqttDevice*>(userdata);
    if (device) {
        device->OnMessage(message);
    }
}

void CMqttDevice::OnConnectCallback(struct mosquitto *mosq, void *userdata, int rc) {
    CMqttDevice *device = static_cast<CMqttDevice*>(userdata);
    if (device) {
        device->OnConnect(rc);
    }
}

void CMqttDevice::OnDisconnectCallback(struct mosquitto *mosq, void *userdata, int rc) {
    CMqttDevice *device = static_cast<CMqttDevice*>(userdata);
    if (device) {
        device->OnDisconnect(rc);
    }
}


CMqttDevice::CMqttDevice(CDescriptor &descriptor) {
    _mosq = nullptr;
    _connected = false;
    _port = 1883;
    _qos = 0;
    _currentOffset = 0;

    const char *modeStr = descriptor.GetFirst();
    const char *host = descriptor.GetNext();
    const char *portStr = descriptor.GetNext();
    const char *topic = descriptor.GetNext();
    const char *qosStr = descriptor.GetNext();
    const char *clientId = descriptor.GetNext();
    const char *userPass = descriptor.GetNext();

    if (modeStr == nullptr || host == nullptr || portStr == nullptr || topic == nullptr) {
        LOGERROR(1, "Error: Wrong MQTT descriptor format (shall be: mode:host:port:topic[:qos[:clientid[:user:pass]]])\n");
        LOGERROR(1, "  mode: SUB or PUB\n");
        LOGERROR(1, "  host: MQTT broker hostname\n");
        LOGERROR(1, "  port: MQTT broker port (1883 or 8883)\n");
        LOGERROR(1, "  topic: MQTT topic (e.g., asterix/raw/048)\n");
        return;
    }

    _mode = ParseMode(modeStr);
    _host = host;
    _port = atoi(portStr);
    _topic = topic;

    if (qosStr != nullptr && strlen(qosStr) > 0) {
        _qos = atoi(qosStr);
        if (_qos < 0 || _qos > 2) {
            LOGWARNING(1, "Invalid QoS %d, defaulting to 0\n", _qos);
            _qos = 0;
        }
    }

    if (clientId != nullptr && strlen(clientId) > 0) {
        _clientId = clientId;
    } else {
        // Generate unique client ID
        char buf[64];
        snprintf(buf, sizeof(buf), "asterix-%d-%ld", getpid(), time(nullptr));
        _clientId = buf;
    }

    if (userPass != nullptr && strlen(userPass) > 0) {
        // Parse user:pass format
        std::string up = userPass;
        size_t colonPos = up.find(':');
        if (colonPos != std::string::npos) {
            _username = up.substr(0, colonPos);
            _password = up.substr(colonPos + 1);
        } else {
            _username = up;
        }
    }

    LOGINFO(gVerbose, "MQTT mode: %s\n", modeStr);
    LOGINFO(gVerbose, "MQTT broker: %s:%d\n", _host.c_str(), _port);
    LOGINFO(gVerbose, "MQTT topic: %s\n", _topic.c_str());
    LOGINFO(gVerbose, "MQTT QoS: %d\n", _qos);
    LOGINFO(gVerbose, "MQTT client ID: %s\n", _clientId.c_str());

    // Initialize Mosquitto
    if (!InitMosquitto()) {
        LOGERROR(1, "Failed to initialize MQTT client\n");
        return;
    }

    _opened = true;
}


CMqttDevice::~CMqttDevice() {
    if (_mosq != nullptr) {
        mosquitto_disconnect(_mosq);
        mosquitto_loop_stop(_mosq, true);
        mosquitto_destroy(_mosq);
        _mosq = nullptr;
    }

    mosquitto_lib_cleanup();

    LOGDEBUG(ZONE_UDPDEVICE, "MQTT device destroyed\n");
}


CMqttDevice::Mode CMqttDevice::ParseMode(const char *modeStr) {
    if (strcasecmp(modeStr, "PUB") == 0 || strcasecmp(modeStr, "PUBLISH") == 0) {
        return MQTT_MODE_PUB;
    }
    // Default to SUB
    return MQTT_MODE_SUB;
}


bool CMqttDevice::InitMosquitto() {
    // Initialize Mosquitto library
    mosquitto_lib_init();

    // Create Mosquitto client
    _mosq = mosquitto_new(_clientId.c_str(), true, this);
    if (_mosq == nullptr) {
        LOGERROR(1, "Failed to create Mosquitto client: %s\n", strerror(errno));
        return false;
    }

    // Set callbacks
    mosquitto_message_callback_set(_mosq, OnMessageCallback);
    mosquitto_connect_callback_set(_mosq, OnConnectCallback);
    mosquitto_disconnect_callback_set(_mosq, OnDisconnectCallback);

    // Set username/password if provided
    if (!_username.empty()) {
        int rc = mosquitto_username_pw_set(_mosq, _username.c_str(),
                                           _password.empty() ? nullptr : _password.c_str());
        if (rc != MOSQ_ERR_SUCCESS) {
            LOGERROR(1, "Failed to set MQTT credentials: %s\n", mosquitto_strerror(rc));
            return false;
        }
    }

    // Connect to broker
    int rc = mosquitto_connect(_mosq, _host.c_str(), _port, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        LOGERROR(1, "Failed to connect to MQTT broker %s:%d: %s\n",
                 _host.c_str(), _port, mosquitto_strerror(rc));
        return false;
    }

    // Start the network loop in a background thread
    rc = mosquitto_loop_start(_mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        LOGERROR(1, "Failed to start MQTT loop: %s\n", mosquitto_strerror(rc));
        return false;
    }

    // Wait for connection (with timeout)
    auto start = std::chrono::steady_clock::now();
    while (!_connected) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 10) {
            LOGERROR(1, "Timeout waiting for MQTT connection\n");
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Subscribe if in SUB mode
    if (_mode == MQTT_MODE_SUB) {
        rc = mosquitto_subscribe(_mosq, nullptr, _topic.c_str(), _qos);
        if (rc != MOSQ_ERR_SUCCESS) {
            LOGERROR(1, "Failed to subscribe to topic %s: %s\n",
                     _topic.c_str(), mosquitto_strerror(rc));
            return false;
        }
        LOGINFO(gVerbose, "MQTT subscribed to %s\n", _topic.c_str());
    }

    return true;
}


void CMqttDevice::OnMessage(const struct mosquitto_message *message) {
    if (message->payloadlen <= 0) {
        return;
    }

    // Copy message to queue
    std::vector<unsigned char> data(
        static_cast<unsigned char*>(message->payload),
        static_cast<unsigned char*>(message->payload) + message->payloadlen
    );

    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _messageQueue.push(std::move(data));
    }
    _queueCondition.notify_one();

    LOGDEBUG(ZONE_UDPDEVICE, "MQTT received %d bytes on topic %s\n",
             message->payloadlen, message->topic);
}


void CMqttDevice::OnConnect(int rc) {
    if (rc == 0) {
        _connected = true;
        LOGINFO(gVerbose, "MQTT connected to %s:%d\n", _host.c_str(), _port);
    } else {
        LOGERROR(1, "MQTT connection failed: %s\n", mosquitto_connack_string(rc));
    }
}


void CMqttDevice::OnDisconnect(int rc) {
    _connected = false;
    if (rc != 0) {
        LOGWARNING(1, "MQTT disconnected unexpectedly: %s\n", mosquitto_strerror(rc));
    } else {
        LOGINFO(gVerbose, "MQTT disconnected\n");
    }
}


bool CMqttDevice::Read(void *data, size_t len) {
    return Read(data, &len);
}


bool CMqttDevice::Read(void *data, size_t *len) {
    if (!_opened || _mosq == nullptr) {
        LOGERROR(1, "Cannot read from MQTT: not initialized\n");
        CountReadError();
        return false;
    }

    if (_mode != MQTT_MODE_SUB) {
        LOGERROR(1, "Cannot read from MQTT in PUB mode\n");
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


bool CMqttDevice::Write(const void *data, size_t len) {
    if (!_opened || _mosq == nullptr) {
        LOGERROR(1, "Cannot write to MQTT: not initialized\n");
        CountWriteError();
        return false;
    }

    if (_mode != MQTT_MODE_PUB) {
        LOGERROR(1, "Cannot write to MQTT in SUB mode\n");
        CountWriteError();
        return false;
    }

    int rc = mosquitto_publish(_mosq, nullptr, _topic.c_str(), len, data, _qos, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        LOGERROR(1, "MQTT publish error: %s\n", mosquitto_strerror(rc));
        CountWriteError();
        return false;
    }

    LOGDEBUG(ZONE_UDPDEVICE, "MQTT published %zu bytes to %s\n", len, _topic.c_str());

    ResetWriteErrors(true);
    return true;
}


bool CMqttDevice::Select(const unsigned int secondsToWait) {
    if (!_opened || _mosq == nullptr) {
        LOGERROR(1, "Cannot select on MQTT: not initialized\n");
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
        _queueCondition.wait(lock, [this] { return !_messageQueue.empty(); });
        return true;
    } else {
        // Wait with timeout
        auto result = _queueCondition.wait_for(
            lock,
            std::chrono::seconds(secondsToWait),
            [this] { return !_messageQueue.empty(); }
        );
        return result;
    }
}


bool CMqttDevice::IoCtrl(const unsigned int command, const void *data, size_t len) {
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
            // Packet processing complete - no action needed
            return true;

        case EAllDone:
            // All processing complete
            return true;

        case EIsLastPacket:
            // MQTT doesn't have a concept of "last packet"
            return false;

        default:
            return false;
    }
}

#endif // HAVE_MQTT
