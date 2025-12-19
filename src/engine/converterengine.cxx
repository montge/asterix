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
 * AUTHORS: Jurica Baricevic, Croatia Control Ltd.
 *
 */
#include <stdio.h>
#include <string.h>

#include "asterix.h"
#include "converterengine.hxx"
#include "channelfactory.hxx"
#include "descriptor.hxx"

CSingleton<CConverterEngine> CConverterEngine::_Instance;


bool CConverterEngine::Initialize(const char *inputChannel, const char *outputChannel[], const unsigned int nOutput,
                                  const unsigned int chFailover) {

    ASSERT(inputChannel);

    CDescriptor inputDescriptor(inputChannel, ";");
    const char *inputDevice = inputDescriptor.GetFirst();
    const char *inputDeviceDescriptor = inputDescriptor.GetNext();
    const char *inputFormat = inputDescriptor.GetNext();
    const char *inputFormatDescriptor = inputDescriptor.GetNext();

    LOGDEBUG(1, "inputDevice(%s)", inputDevice);
    LOGDEBUG(1, "inputDeviceDescriptor(%s)", inputDeviceDescriptor);
    LOGDEBUG(1, "inputFormatDescriptor(%s)", inputFormatDescriptor);

    // Check input channel parameters consistency
    if ((inputDevice == nullptr) || (inputDeviceDescriptor == nullptr) || (inputFormat == nullptr)) {
        LOGERROR(1, "Input channel descriptor must be in the following format: \n\""
                    "<device> <device_descriptor> <format> [format_descriptor]\"\n");
        return false;
    }

    LOGDEBUG(1, "Input format: %s\n", inputFormat);

    // Create input channel
    if (!CChannelFactory::Instance()->CreateInputChannel(inputDevice, inputDeviceDescriptor, inputFormat,
                                                         inputFormatDescriptor)) {
        LOGERROR(1, "Input channel initialization failed.\n");
        return false;
    }

    // Process output channels
    for (unsigned int i = 0; i < nOutput; i++) {
        // TODO: special initialization of Failover channels

        ASSERT(outputChannel[i]);
        CDescriptor outputDescriptor(outputChannel[i], " ");
        const char *outputDevice = outputDescriptor.GetFirst();
        const char *outputDeviceDescriptor = outputDescriptor.GetNext();
        const char *outputFormat = outputDescriptor.GetNext();
        const char *outputFormatDescriptor = outputDescriptor.GetNext();
        const char *outputHeartbeat = outputDescriptor.GetNext();

        // Check output channel parameters consistency
        if ((outputDevice == nullptr) || (outputDeviceDescriptor == nullptr) || (outputFormat == nullptr)) {
            LOGERROR(1, "Output channel descriptor must be in the following format: \n\""
                        "<device> <device_descriptor> <format> [format_descriptor] [heartbeat]\"\n");
            return false;
        }

        LOGDEBUG(1, "Output format: %s\n", outputFormat);

        // Create output channel
        if (!CChannelFactory::Instance()->CreateOutputChannel(outputDevice, outputDeviceDescriptor, outputFormat,
                                                              outputFormatDescriptor, i >= chFailover,
                                                              outputHeartbeat)) {
            LOGERROR(1, "Output channel initialization failed.\n");
            return false;
        }
    }

    return true;
}


// Helper: Wait for packet with heartbeat processing
void CConverterEngine::waitForPacketWithHeartbeat(unsigned int nChannels) {
    bool packetReceived;
    do {
        packetReceived = CChannelFactory::Instance()->WaitForPacket(gHeartbeat);

        // HeartbeatProcessing for all output channels is called *AT LEAST* every gHeartBeat
        for (unsigned int i = 0; i < nChannels; ++i) {
            if (!CChannelFactory::Instance()->HeartbeatProcessing(i)) {
                LOGERROR(1, "Heartbeat() failed.\n");
            }
        }
    } while (!packetReceived);
}

// Helper: Handle packet reading, returns true if packet read OK
bool CConverterEngine::handlePacketRead(bool &noMoreData) {
    if (CChannelFactory::Instance()->ReadPacket()) {
        return true;
    }

    int sts = ProcessStatus();
    noMoreData = (sts & STS_NO_DATA) != 0;

    if (noMoreData) {
        LOGINFO(1, "No more data available on input channel.\n");
    }
    return false;
}

// Helper: Handle packet processing, returns true to continue main loop
bool CConverterEngine::handlePacketProcess(bool packetOk, bool noMoreData, bool &discard) {
    if (!packetOk || noMoreData) {
        return true;
    }

    if (!CChannelFactory::Instance()->ProcessPacket(discard)) {
        LOGERROR(1, "ProcessPacket() failed.\n");
        return (ProcessStatus() & (STS_FAIL_INPUT | STS_FAIL_DATA)) != 0;
    }
    return true;
}

// Helper: Dispatch to normal (non-failover) output channels
void CConverterEngine::dispatchToNormalChannels(unsigned int nChannels, bool noMoreData, bool packetOk) {
    for (unsigned int i = 0; i < nChannels; ++i) {
        if (CChannelFactory::Instance()->IsFailoverOutputChannel(i)) {
            break; // Stop at first failover channel
        }

        if (noMoreData) {
            if (!CChannelFactory::Instance()->IoCtrl(static_cast<int>(i), CBaseDevice::EAllDone)) {
                LOGERROR(1, "IoCtrl() failed.\n");
            }
        } else if (packetOk) {
            if (!CChannelFactory::Instance()->WritePacket(i)) {
                LOGERROR(1, "WritePacket() failed.\n");
            }
        }
    }
}

// Helper: Handle failover output channels with automatic switching
void CConverterEngine::dispatchToFailoverChannels(unsigned int nChannels) {
    unsigned int ch = CChannelFactory::Instance()->GetActiveFailoverOutputChannel();
    unsigned int startCh = ch;

    while (ch < nChannels) {
        if (CChannelFactory::Instance()->WritePacket(ch)) {
            break; // Success
        }

        LOGERROR(1, "The current packet has been lost for failover output channel %d\n", static_cast<int>(ch));

        int sts = ProcessStatus();

        if (sts & STS_FAIL_OUTPUT) {
            ch = CChannelFactory::Instance()->GetNextFailoverOutputChannel();
            LOGNOTIFY(gVerbose, "Switching to failover output channel: %d\n", static_cast<int>(ch));

            if (ch == startCh) {
                break; // Cycled through all failover channels
            }
        }

        if (sts & STS_FAIL_INPUT) {
            break; // Input channel reset, discard current packet
        }
    }
}

// TODO:
// - call appropriate method (todo- write method) to analyze if the packet needs to be discarded
// - enable disconnection/reconnection with server (check CreateOutputChannel).
//
void CConverterEngine::Start() {
    unsigned int nChannels = CChannelFactory::Instance()->GetNOutputChannels();
    bool discard = false;

    LOGNOTIFY(gVerbose, "Converter Engine Started.\n");

    while (true) {
        // 1. Wait for incoming packet on input channel
        waitForPacketWithHeartbeat(nChannels);

        // 1a. Check if there is more data
        int sts = ProcessStatus();
        if (sts & STS_NO_DATA) {
            break;
        }

        // 2. Read the incoming packet
        bool noMoreData = false;
        bool packetOk = handlePacketRead(noMoreData);

        if (noMoreData) {
            break;
        }
        if (!packetOk && (ProcessStatus() & (STS_FAIL_INPUT | STS_FAIL_DATA))) {
            continue;
        }

        // 3. Process the packet
        if (!handlePacketProcess(packetOk, noMoreData, discard)) {
            continue;
        }

        // 4. Dispatch to output channels if not discarded
        if (gForceRouting || !discard) {
            dispatchToNormalChannels(nChannels, noMoreData, packetOk);
            dispatchToFailoverChannels(nChannels);
        }
    }
}


int CConverterEngine::ProcessStatus() {
    int sts = CChannelFactory::Instance()->GetStatus();

    if ((sts & STS_FAIL_INPUT) || (sts & STS_FAIL_DATA)) {
        LOGWARNING(1, "Resetting input channel due to excessive errors or data errors\n");
        if (!CChannelFactory::Instance()->ResetInputChannel()) {
            LOGERROR(1, "Failed to reset input channel.\n");
        }
    }

    unsigned int activeFOC = CChannelFactory::Instance()->GetActiveFailoverOutputChannel();

    if (sts & STS_FAIL_OUTPUT) {
        LOGWARNING(1, "Resetting output channel %d due to excessive errors\n", static_cast<int>(activeFOC));
        if (!CChannelFactory::Instance()->ResetOutputChannel(activeFOC)) {
            LOGERROR(1, "Failed to reset output channel %d.\n", static_cast<int>(activeFOC));
        }
    }

    return sts;
}
