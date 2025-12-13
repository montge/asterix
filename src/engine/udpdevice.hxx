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

#ifndef UDPDEVICE_HXX__
#define UDPDEVICE_HXX__

#ifdef _WIN32
  // Windows socket headers
  #include <winsock2.h>
  #include <ws2tcpip.h>
  // Link with Winsock library (ws2_32.lib)
  #pragma comment(lib, "ws2_32.lib")
#else
  // POSIX socket headers
  #include <sys/socket.h>
  #include <sys/select.h> // fd_set
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif

#include <vector>

#include "basedevice.hxx"
#include "descriptor.hxx"

/*
 * MSG_NOSIGNAL is not defined on older macOS versions
 * but has an equivalent (SO_NOSIGPIPE)
 * Newer macOS (14+) defines MSG_NOSIGNAL in sys/socket.h, so check first
 */
#ifdef __APPLE__
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif
#endif

#define MAX_UDP_PACKET_SIZE     3000

/**
 * @class CUdpDevice
 *
 * @brief The UDP multicast device.
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 *        <CDescriptor>
 */
class CUdpDevice : public CBaseDevice {
private:
    bool _server;
    struct sockaddr_in _mcastAddr;
    struct in_addr _interfaceAddr;
    struct in_addr _sourceAddr;
    int _port;
    std::vector<int> _socketDesc;
    fd_set _descToRead;
    fd_set _descToReadTemplate;  // PERFORMANCE: Persistent template to avoid rebuilding on every Select()
    int _countToRead;
    int _maxValSocketDesc;

private:
    bool InitServer(int socketDesc);

    bool InitClient(int socketDesc);

public:

    /**
     * Class constructor which uses descriptor
     */
    explicit CUdpDevice(CDescriptor &descriptor);

    /**
     * Class destructor.
     */
    virtual ~CUdpDevice();

    virtual bool Read(void *data, size_t len);

    virtual bool Read(void *data, size_t *len);

    virtual bool Write(const void *data, size_t len);

    virtual bool Select(const unsigned int secondsToWait);

    virtual bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0) { return false; }

    virtual bool IsPacketDevice() { return true; }

    virtual unsigned int
    MaxPacketSize() { return MAX_UDP_PACKET_SIZE; } // return maximal packet size (only for packet devices)
    virtual unsigned int BytesLeftToRead() { return 0; } // return number of bytes left to read or 0 if unknown

private:
    void Init(const char *mcastAddress, const char *interfaceAddress, const char *srcAddress, const int port,
              const bool server);

};

#endif

