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
 * AUTHORS: Claude Code (ASTERIX Cyclone DDS integration)
 *
 */

#ifndef CYCLONEDDSDEVICE_HXX__
#define CYCLONEDDSDEVICE_HXX__

#ifdef HAVE_CYCLONEDDS

#include <dds/dds.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>
#include <atomic>

#include "basedevice.hxx"
#include "descriptor.hxx"

#define MAX_DDS_MESSAGE_SIZE    65536

/**
 * @class CCycloneDdsDevice
 *
 * @brief Cyclone DDS device for real-time, deterministic ASTERIX data distribution.
 *
 * Implements Eclipse Cyclone DDS pub/sub for safety-critical systems requiring
 * microsecond latency and configurable QoS policies.
 *
 * Descriptor format: mode:domain:topic[:qos]
 *   - mode: PUB (publisher) or SUB (subscriber)
 *   - domain: DDS domain ID (0-232)
 *   - topic: Topic name (e.g., "AsterixData")
 *   - qos: Optional QoS profile (reliable, besteffort, transient)
 *
 * Examples:
 *   SUB:0:AsterixData                - Subscribe on domain 0
 *   SUB:0:AsterixData:reliable       - Subscribe with reliable QoS
 *   PUB:0:AsterixCat048:transient    - Publish with transient durability
 *
 * QoS Profiles:
 *   - besteffort: Best-effort reliability (default)
 *   - reliable: Reliable delivery with acknowledgments
 *   - transient: Transient local durability (late joiners get data)
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 *        <CDescriptor>
 */
class CCycloneDdsDevice : public CBaseDevice {
public:
    /**
     * Mode enumeration
     */
    enum Mode {
        DDS_MODE_PUB,   // Publisher mode
        DDS_MODE_SUB    // Subscriber mode
    };

    /**
     * QoS profile enumeration
     */
    enum QosProfile {
        QOS_BEST_EFFORT,    // Best-effort reliability
        QOS_RELIABLE,       // Reliable delivery
        QOS_TRANSIENT       // Transient local durability
    };

private:
    Mode _mode;                          // PUB or SUB mode
    int _domainId;                       // DDS domain ID
    std::string _topicName;              // Topic name
    QosProfile _qosProfile;              // QoS profile

    // DDS entities
    std::shared_ptr<dds::domain::DomainParticipant> _participant;
    std::shared_ptr<dds::topic::Topic<dds::core::BytesTopicType>> _topic;
    std::shared_ptr<dds::pub::DataWriter<dds::core::BytesTopicType>> _writer;
    std::shared_ptr<dds::sub::DataReader<dds::core::BytesTopicType>> _reader;

    // Message queue for received data (SUB mode)
    std::queue<std::vector<unsigned char>> _messageQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;

    // Current message being read
    std::vector<unsigned char> _currentMessage;
    size_t _currentOffset;

    // Listener thread for SUB mode
    std::unique_ptr<std::thread> _listenerThread;
    std::atomic<bool> _running;

private:
    /**
     * Initialize DDS publisher
     */
    bool InitPublisher();

    /**
     * Initialize DDS subscriber
     */
    bool InitSubscriber();

    /**
     * Parse mode string to enum
     */
    Mode ParseMode(const char *modeStr);

    /**
     * Parse QoS profile string to enum
     */
    QosProfile ParseQosProfile(const char *qosStr);

    /**
     * Configure QoS based on profile
     */
    dds::pub::qos::DataWriterQos GetWriterQos();
    dds::sub::qos::DataReaderQos GetReaderQos();

    /**
     * Listener thread function
     */
    void ListenerLoop();

public:

    /**
     * Class constructor which uses descriptor
     *
     * @param descriptor Descriptor with format: mode:domain:topic[:qos]
     */
    CCycloneDdsDevice(CDescriptor &descriptor);

    /**
     * Class destructor.
     */
    virtual ~CCycloneDdsDevice();

    /**
     * Read data from DDS (exact length)
     *
     * @param data Buffer to store received data
     * @param len Number of bytes to read
     * @return true if successful, false on error
     */
    virtual bool Read(void *data, size_t len);

    /**
     * Read data from DDS (variable length)
     *
     * @param data Buffer to store received data
     * @param len Input: max buffer size, Output: actual bytes read
     * @return true if successful, false on error
     */
    virtual bool Read(void *data, size_t *len);

    /**
     * Write data to DDS (publish)
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
     * Returns true - DDS messages are packet-oriented
     */
    virtual bool IsPacketDevice() { return true; }

    /**
     * Returns maximum message size supported
     */
    virtual unsigned int MaxPacketSize() { return MAX_DDS_MESSAGE_SIZE; }

    /**
     * Returns 0 - DDS doesn't support "bytes left to read" concept
     */
    virtual unsigned int BytesLeftToRead() { return 0; }
};

#endif // HAVE_CYCLONEDDS

#endif // CYCLONEDDSDEVICE_HXX__
