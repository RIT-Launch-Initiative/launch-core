#ifndef LINUX_UDP_SOCKET_DEVICE_H
#define LINUX_UDP_SOCKET_DEVICE_H

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "device/Device.h"
#include "device/SocketDevice.h"
#include "queue/allocated_queue.h"
#include "pool/pool.h"
#include "return.h"

// how many packets we buffer
static const size_t BUFFER_SIZE = 10;

// the size of a packet we buffer
static const size_t PACKET_SIZE = 1472; // max size of a UDP payload over ethernet

/// @brief Linux UDP socket device
class LinuxUdpSocketDevice : public SocketDevice {
public:
    /// @brief network packet
    typedef struct {
        uint8_t buff[PACKET_SIZE];
        size_t len;
        addr_t src;
    } Packet;

    /// @brief constructor
    LinuxUdpSocketDevice() : m_sockFd(-1), m_prealloc(NULL),
                             SocketDevice("Linux UDP Socket Device") {};

    RetType init() {
        m_sockFd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
        if(m_sockFd < 0) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    RetType obtain() {
        return RET_SUCCESS;
    }

    RetType release() {
        return RET_SUCCESS;
    }

    RetType poll() {
        Packet* packet;

        if(m_prealloc) {
            packet = m_prealloc;
            m_prealloc = NULL;
        } else {
            packet = m_queue.push();

            if(NULL == packet) {
                // no space on queue
                return RET_ERROR;
            }
        }

        struct sockaddr_in addr;
        socklen_t addr_len;
        ssize_t len = recvfrom(m_sockFd, packet->buff, PACKET_SIZE,
                               0, (struct sockaddr*)&addr, &addr_len);
        if(len <= 0 || addr_len != sizeof(addr)) {
            // error receiving
            m_prealloc = packet; // use this packet next time
            return RET_ERROR;
        }

        packet->len = len;
        packet->src.addr = ntohl(addr.sin_addr.s_addr);
        packet->src.port = ntohs(addr.sin_port);
        return RET_SUCCESS;
    }

    RetType bind(addr_t* addr) {
        struct sockaddr_in sock_addr;
        sock_addr.sin_addr.s_addr = htonl(addr->addr);
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_port = htons(addr->port);

        if(::bind(m_sockFd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) != 0) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    RetType subscribe(addr_t* addr) {
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = htonl(addr->addr);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if(setsockopt(m_sockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    RetType sendmsg(msg_t* msg) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(msg->addr.addr);
        addr.sin_port = htons(msg->addr.port);

        if(sendto(m_sockFd, (void*)(msg->payload), msg->payload_len,
                  0, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            // failed to send
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    // only non-blocking for now
    RetType recvmsg(msg_t* msg) {
        Packet* packet = m_queue.peek();
        m_queue.pop();

        if(packet == NULL) {
            // nothing popped
            return RET_ERROR;
        }

        if(msg->payload_len > packet->len) {
            msg->payload_len = packet->len;
        } // otherwise truncate and don't copy all of the packet

        memcpy(msg->payload, packet->buff, msg->payload_len);
        msg->addr = packet->src;
        return RET_SUCCESS;
    }

    /// @brief close the socket
    /// @return
    RetType close() {
        if(-1 == m_sockFd) {
            // never opened
            return RET_SUCCESS;
        }

        if(-1 == ::close(m_sockFd)) {
            return RET_ERROR;
        }
    }

    /// @brief get how many packets are available
    /// @return the number of available packets
    size_t available() {
        size_t size = m_queue.size();

        if(m_prealloc) {
            size--;
        }

        return size;
    }

private:
    int m_sockFd;
    alloc::Queue<Packet, BUFFER_SIZE> m_queue;
    Packet* m_prealloc; // preallocated packet if we fail a push
};

#endif
