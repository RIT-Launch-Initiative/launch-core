#ifndef LINUX_UDP_SOCKET_POOL_H
#define LINUX_UDP_SOCKET_POOL_H

#include <stdint.h>

#include "device/SocketPool.h"
#include "device/platforms/linux/LinuxUdpSocket.h"

/// @brief socket pool of Linux UDP sockets
/// @tparam SIZE    number of sockets
template <const size_t SIZE>
class LinuxUdpSocketPool : public alloc::SocketPool<LinuxUdpSocket, SIZE> {

using alloc::SocketPool<LinuxUdpSocket, SIZE>::m_socks;

public:
    /// @brief constructor
    LinuxUdpSocketPool(const char* name) : alloc::SocketPool<LinuxUdpSocket, SIZE>(name) {};

    /// @brief poll all allocated sockets
    RetType poll() {
        RetType ret = RET_SUCCESS;
        RetType temp;

        QueueIterator<LinuxUdpSocket> it = m_socks.iterator();
        LinuxUdpSocket* sock;

        while(sock = *it) {
            temp = sock->buffer();
            if(temp != RET_SUCCESS) {
                ret = temp;
            }

            ++it;
        }

        return ret;
    }
};

#endif
