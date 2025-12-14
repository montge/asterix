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

#ifndef MQTTDEVICE_HXX__
#define MQTTDEVICE_HXX__

#ifdef HAVE_MQTT

#include <mosquitto.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "basedevice.hxx"
#include "descriptor.hxx"

#define MAX_MQTT_MESSAGE_SIZE    65536

/**
 * @class CMqttDevice
 *
 * @brief MQTT device for IoT and cloud messaging transport.
 *
 * Supports subscribing to MQTT topics to receive ASTERIX data, or
 * publishing ASTERIX data to MQTT topics.
 *
 * Descriptor format: mode:host:port:topic[:qos[:clientid[:user:pass]]]
 *   - mode: SUB (subscribe) or PUB (publish)
 *   - host: MQTT broker hostname
 *   - port: MQTT broker port (usually 1883 or 8883 for TLS)
 *   - topic: MQTT topic (e.g., asterix/raw/048)
 *   - qos: Optional, QoS level 0, 1, or 2 (default: 0)
 *   - clientid: Optional, client ID (default: auto-generated)
 *   - user:pass: Optional, username and password
 *
 * Examples:
 *   SUB:localhost:1883:asterix/#                - Subscribe to all asterix topics
 *   SUB:broker.example.com:1883:asterix/cat048:1 - Subscribe with QoS 1
 *   PUB:localhost:1883:asterix/raw:0:asterix-pub - Publish with client ID
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 *        <CDescriptor>
 */
class CMqttDevice : public CBaseDevice {
public:
    /**
     * Mode enumeration
     */
    enum Mode {
        MQTT_MODE_SUB,   // Subscriber mode
        MQTT_MODE_PUB    // Publisher mode
    };

private:
    struct mosquitto *_mosq;      // Mosquitto client instance
    Mode _mode;                    // SUB or PUB mode
    std::string _host;             // Broker hostname
    int _port;                     // Broker port
    std::string _topic;            // MQTT topic
    int _qos;                      // QoS level (0, 1, 2)
    std::string _clientId;         // Client ID
    std::string _username;         // Optional username
    std::string _password;         // Optional password

    // Message queue for received messages (SUB mode)
    std::queue<std::vector<unsigned char>> _messageQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;
    bool _connected;

    // Current message being read
    std::vector<unsigned char> _currentMessage;
    size_t _currentOffset;

private:
    /**
     * Initialize the Mosquitto client
     */
    bool InitMosquitto();

    /**
     * Parse mode string to enum
     */
    Mode ParseMode(const char *modeStr);

    /**
     * Static callback for message received
     */
    static void OnMessageCallback(struct mosquitto *mosq, void *userdata,
                                  const struct mosquitto_message *message);

    /**
     * Static callback for connection
     */
    static void OnConnectCallback(struct mosquitto *mosq, void *userdata, int rc);

    /**
     * Static callback for disconnect
     */
    static void OnDisconnectCallback(struct mosquitto *mosq, void *userdata, int rc);

    /**
     * Instance method for handling received messages
     */
    void OnMessage(const struct mosquitto_message *message);

    /**
     * Instance method for handling connection
     */
    void OnConnect(int rc);

    /**
     * Instance method for handling disconnect
     */
    void OnDisconnect(int rc);

public:

    /**
     * Class constructor which uses descriptor
     *
     * @param descriptor Descriptor with format: mode:host:port:topic[:qos[:clientid[:user:pass]]]
     */
    explicit CMqttDevice(CDescriptor &descriptor);

    /**
     * Class destructor.
     */
    ~CMqttDevice() override;

    /**
     * Read data from MQTT message queue (exact length)
     *
     * @param data Buffer to store received data
     * @param len Number of bytes to read
     * @return true if successful, false on error
     */
    bool Read(void *data, size_t len) override;

    /**
     * Read data from MQTT message queue (variable length)
     *
     * @param data Buffer to store received data
     * @param len Input: max buffer size, Output: actual bytes read
     * @return true if successful, false on error
     */
    bool Read(void *data, size_t *len) override;

    /**
     * Write data to MQTT topic (publish)
     *
     * @param data Data to send
     * @param len Number of bytes to send
     * @return true if successful, false on error
     */
    bool Write(const void *data, size_t len) override;

    /**
     * Wait for message availability
     *
     * @param secondsToWait Timeout in seconds (0 = wait indefinitely)
     * @return true if message available, false on timeout or error
     */
    bool Select(const unsigned int secondsToWait) override;

    /**
     * IO control operations
     *
     * @param command Control command (EReset, EPacketDone, etc.)
     * @param data Optional data parameter
     * @param len Optional length parameter
     * @return true if successful, false otherwise
     */
    bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0) override;

    /**
     * Returns true - MQTT messages are packet-oriented
     */
    bool IsPacketDevice() override { return true; }

    /**
     * Returns maximum message size supported
     */
    unsigned int MaxPacketSize() override { return MAX_MQTT_MESSAGE_SIZE; }

    /**
     * Returns 0 - MQTT doesn't support "bytes left to read" concept
     */
    unsigned int BytesLeftToRead() override { return 0; }

    /**
     * Check if connected to broker
     */
    bool IsConnected() const { return _connected; }
};

#endif // HAVE_MQTT

#endif // MQTTDEVICE_HXX__
