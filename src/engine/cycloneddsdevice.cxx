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

#ifdef HAVE_CYCLONEDDS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <chrono>

// Cross-platform compatibility layer
#ifdef _WIN32
#include "win32_compat.h"
#endif

// Local includes
#include "asterix.h"
#include "cycloneddsdevice.hxx"
#include "descriptor.hxx"


CCycloneDdsDevice::CCycloneDdsDevice(CDescriptor &descriptor) {
    _domainId = 0;
    _qosProfile = QOS_BEST_EFFORT;
    _currentOffset = 0;
    _running = false;

    const char *modeStr = descriptor.GetFirst();
    const char *domainStr = descriptor.GetNext();
    const char *topicStr = descriptor.GetNext();
    const char *qosStr = descriptor.GetNext();

    if (modeStr == nullptr || domainStr == nullptr || topicStr == nullptr) {
        LOGERROR(1, "Error: Wrong Cyclone DDS descriptor format (shall be: mode:domain:topic[:qos])\n");
        LOGERROR(1, "  mode: PUB or SUB\n");
        LOGERROR(1, "  domain: DDS domain ID (0-232)\n");
        LOGERROR(1, "  topic: Topic name (e.g., AsterixData)\n");
        LOGERROR(1, "  qos: Optional QoS profile (besteffort, reliable, transient)\n");
        return;
    }

    _mode = ParseMode(modeStr);
    _domainId = atoi(domainStr);
    _topicName = topicStr;

    if (qosStr != nullptr && strlen(qosStr) > 0) {
        _qosProfile = ParseQosProfile(qosStr);
    }

    // Validate domain ID
    if (_domainId < 0 || _domainId > 232) {
        LOGERROR(1, "Error: Invalid DDS domain ID %d (must be 0-232)\n", _domainId);
        return;
    }

    LOGINFO(gVerbose, "Cyclone DDS mode: %s\n", modeStr);
    LOGINFO(gVerbose, "Cyclone DDS domain: %d\n", _domainId);
    LOGINFO(gVerbose, "Cyclone DDS topic: %s\n", _topicName.c_str());
    LOGINFO(gVerbose, "Cyclone DDS QoS: %s\n",
            _qosProfile == QOS_RELIABLE ? "reliable" :
            _qosProfile == QOS_TRANSIENT ? "transient" : "besteffort");

    // Initialize based on mode
    bool success = false;
    if (_mode == DDS_MODE_PUB) {
        success = InitPublisher();
    } else {
        success = InitSubscriber();
    }

    if (!success) {
        LOGERROR(1, "Failed to initialize Cyclone DDS device\n");
        return;
    }

    _opened = true;
}


CCycloneDdsDevice::~CCycloneDdsDevice() {
    // Stop listener thread
    _running = false;
    _queueCondition.notify_all();

    if (_listenerThread && _listenerThread->joinable()) {
        _listenerThread->join();
    }

    // DDS entities will be cleaned up by shared_ptr destructors

    LOGDEBUG(ZONE_UDPDEVICE, "Cyclone DDS device destroyed\n");
}


CCycloneDdsDevice::Mode CCycloneDdsDevice::ParseMode(const char *modeStr) {
    if (strcasecmp(modeStr, "PUB") == 0 || strcasecmp(modeStr, "PUBLISH") == 0 ||
        strcasecmp(modeStr, "PUBLISHER") == 0) {
        return DDS_MODE_PUB;
    }
    // Default to SUB
    return DDS_MODE_SUB;
}


CCycloneDdsDevice::QosProfile CCycloneDdsDevice::ParseQosProfile(const char *qosStr) {
    if (strcasecmp(qosStr, "reliable") == 0 || strcasecmp(qosStr, "rel") == 0) {
        return QOS_RELIABLE;
    } else if (strcasecmp(qosStr, "transient") == 0 || strcasecmp(qosStr, "trans") == 0) {
        return QOS_TRANSIENT;
    }
    // Default to best-effort
    return QOS_BEST_EFFORT;
}


dds::pub::qos::DataWriterQos CCycloneDdsDevice::GetWriterQos() {
    dds::pub::qos::DataWriterQos qos;

    switch (_qosProfile) {
        case QOS_RELIABLE:
            qos << dds::core::policy::Reliability::Reliable();
            break;
        case QOS_TRANSIENT:
            qos << dds::core::policy::Reliability::Reliable();
            qos << dds::core::policy::Durability::TransientLocal();
            break;
        case QOS_BEST_EFFORT:
        default:
            qos << dds::core::policy::Reliability::BestEffort();
            break;
    }

    return qos;
}


dds::sub::qos::DataReaderQos CCycloneDdsDevice::GetReaderQos() {
    dds::sub::qos::DataReaderQos qos;

    switch (_qosProfile) {
        case QOS_RELIABLE:
            qos << dds::core::policy::Reliability::Reliable();
            break;
        case QOS_TRANSIENT:
            qos << dds::core::policy::Reliability::Reliable();
            qos << dds::core::policy::Durability::TransientLocal();
            break;
        case QOS_BEST_EFFORT:
        default:
            qos << dds::core::policy::Reliability::BestEffort();
            break;
    }

    return qos;
}


bool CCycloneDdsDevice::InitPublisher() {
    try {
        // Create DomainParticipant
        _participant = std::make_shared<dds::domain::DomainParticipant>(_domainId);

        // Create Topic using AsterixPayload type for ASTERIX data
        _topic = std::make_shared<dds::topic::Topic<AsterixDDS::AsterixPayload>>(
            *_participant, _topicName);

        // Create Publisher with default QoS
        dds::pub::Publisher publisher(*_participant);

        // Create DataWriter with configured QoS
        _writer = std::make_shared<dds::pub::DataWriter<AsterixDDS::AsterixPayload>>(
            publisher, *_topic, GetWriterQos());

        LOGINFO(gVerbose, "Cyclone DDS publisher initialized on domain %d, topic %s\n",
                _domainId, _topicName.c_str());

        return true;

    } catch (const dds::core::Exception &e) {
        LOGERROR(1, "Cyclone DDS publisher initialization failed: %s\n", e.what());
        return false;
    }
}


bool CCycloneDdsDevice::InitSubscriber() {
    try {
        // Create DomainParticipant
        _participant = std::make_shared<dds::domain::DomainParticipant>(_domainId);

        // Create Topic using AsterixPayload type for ASTERIX data
        _topic = std::make_shared<dds::topic::Topic<AsterixDDS::AsterixPayload>>(
            *_participant, _topicName);

        // Create Subscriber with default QoS
        dds::sub::Subscriber subscriber(*_participant);

        // Create DataReader with configured QoS
        _reader = std::make_shared<dds::sub::DataReader<AsterixDDS::AsterixPayload>>(
            subscriber, *_topic, GetReaderQos());

        // Start listener thread
        _running = true;
        _listenerThread = std::make_unique<std::thread>(&CCycloneDdsDevice::ListenerLoop, this);

        LOGINFO(gVerbose, "Cyclone DDS subscriber initialized on domain %d, topic %s\n",
                _domainId, _topicName.c_str());

        return true;

    } catch (const dds::core::Exception &e) {
        LOGERROR(1, "Cyclone DDS subscriber initialization failed: %s\n", e.what());
        return false;
    }
}


void CCycloneDdsDevice::ListenerLoop() {
    while (_running) {
        try {
            // Wait for data with timeout
            dds::core::cond::WaitSet waitset;
            dds::sub::cond::ReadCondition readCond(*_reader,
                dds::sub::status::DataState::any());
            waitset.attach_condition(readCond);

            auto conditions = waitset.wait(dds::core::Duration::from_secs(1));

            if (!_running) break;

            // Read available samples
            auto samples = _reader->take();
            for (const auto &sample : samples) {
                if (sample.info().valid()) {
                    const auto &payload = sample.data();
                    // Extract raw data from AsterixPayload
                    std::vector<unsigned char> msg(payload.data().begin(), payload.data().end());

                    {
                        std::lock_guard<std::mutex> lock(_queueMutex);
                        _messageQueue.push(std::move(msg));
                    }
                    _queueCondition.notify_one();

                    LOGDEBUG(ZONE_UDPDEVICE, "Cyclone DDS received %zu bytes (cat %u)\n",
                             payload.data().size(), payload.category());
                }
            }

        } catch (const dds::core::TimeoutError &) {
            // Timeout is normal, continue loop
            continue;
        } catch (const dds::core::Exception &e) {
            if (_running) {
                LOGERROR(1, "Cyclone DDS read error: %s\n", e.what());
            }
            break;
        }
    }
}


bool CCycloneDdsDevice::Read(void *data, size_t len) {
    // Delegate to the variable-length Read implementation
    // Note: data is void* to match CBaseDevice interface - actual type is unsigned char*
    unsigned char* buffer = static_cast<unsigned char*>(data);
    size_t bytesToRead = len;
    return Read(buffer, &bytesToRead);
}


bool CCycloneDdsDevice::Read(void *data, size_t *len) {
    if (!_opened) {
        LOGERROR(1, "Cannot read from Cyclone DDS: not initialized\n");
        CountReadError();
        return false;
    }

    if (_mode != DDS_MODE_SUB) {
        LOGERROR(1, "Cannot read from Cyclone DDS in PUB mode\n");
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


bool CCycloneDdsDevice::Write(const void *data, size_t len) {
    if (!_opened) {
        LOGERROR(1, "Cannot write to Cyclone DDS: not initialized\n");
        CountWriteError();
        return false;
    }

    if (_mode != DDS_MODE_PUB) {
        LOGERROR(1, "Cannot write to Cyclone DDS in SUB mode\n");
        CountWriteError();
        return false;
    }

    try {
        // Create AsterixPayload sample from raw data
        AsterixDDS::AsterixPayload sample;

        // Copy raw bytes to payload
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        sample.data(std::vector<uint8_t>(bytes, bytes + len));

        // Extract category from ASTERIX data block header (first byte)
        if (len > 0) {
            sample.category(static_cast<uint16_t>(bytes[0]));
        }

        // Set timestamp (microseconds since epoch)
        sample.timestamp(static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()));

        // Publish
        _writer->write(sample);

        LOGDEBUG(ZONE_UDPDEVICE, "Cyclone DDS published %zu bytes (cat %u)\n",
                 len, sample.category());

        ResetWriteErrors(true);
        return true;

    } catch (const dds::core::Exception &e) {
        LOGERROR(1, "Cyclone DDS write error: %s\n", e.what());
        CountWriteError();
        return false;
    }
}


bool CCycloneDdsDevice::Select(const unsigned int secondsToWait) {
    if (!_opened) {
        LOGERROR(1, "Cannot select on Cyclone DDS: not initialized\n");
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
        _queueCondition.wait(lock, [this] {
            return !_messageQueue.empty() || !_running;
        });
        return !_messageQueue.empty();
    } else {
        // Wait with timeout
        auto result = _queueCondition.wait_for(
            lock,
            std::chrono::seconds(secondsToWait),
            [this] { return !_messageQueue.empty() || !_running; }
        );
        return result && !_messageQueue.empty();
    }
}


bool CCycloneDdsDevice::IoCtrl(const unsigned int command, const void *data, size_t len) {
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
            _running = false;
            _queueCondition.notify_all();
            return true;

        case EIsLastPacket:
            // DDS doesn't have a concept of "last packet"
            return !_running;

        default:
            return false;
    }
}

#endif // HAVE_CYCLONEDDS
