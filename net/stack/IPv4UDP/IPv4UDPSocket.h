/*******************************************************************************
*
*  Name: IPv4UDPSocket.h
*
*  Purpose: Defines a socket for the IPv4/UDP stack.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/

#ifndef IPV4_UDP_SOCKET_H
#define IPV4_UDP_SOCKET_H

#include <stdint.h>

#include "net/udp/udp.h"
#include "net/udp/UDPRouter.h"
#include "net/ipv4/ipv4.h"
#include "net/ipv4/IPv4Router.h"
#include "net/eth/eth.h"
#include "net/eth/EthLayer.h"
#include "return.h"
#include "net/network_layer/NetworkLayer.h"
#include "queue/allocated_queue.h"
#include "net/packet/Packet.h"
#include "pool/pool.h"
#include "sched/macros/macros.h"

// TODO add subscribing to multicast address somehow
//      maybe just bind to multicast address (or 0.0.0.0)
//      and make it a manual route?
class IPv4UDPSocket : public NetworkLayer {
public:
    /// @brief address type
    typedef struct {
        uint8_t ip[4];      // IPv4 address in big endian order
        uint16_t port;      // port number in big endian order
    } addr_t;


    /// @brief MTU size with no headers
    static const size_t MTU_NO_HEADERS = eth::MAX_FRAME_SIZE;


    /// @brief header size (combined)
    static const size_t HEADERS_SIZE = sizeof(eth::EthHeader_t) + \
                                       sizeof(ipv4::IPv4Header_t) + \
                                       sizeof(udp::UDP_HEADER_T);

    /// @brief MTU size
    /// NOTE: this is the maximum payload size excluding the Ethernet FCS
    ///       headers + payload + FCS should add up to MTU_NO_HEADERS
    static const size_t MTU = MTU_NO_HEADERS - HEADERS_SIZE - eth::FCS_LEN;


    /// @brief set the UDP layer to interact with
    /// @return
    /// NOTE: must be set before using any other functions!
    void set_udp(udp::UDPRouter* udp) {
        m_udp = udp;
    }

    /// @brief bind a socket to send/receive from a port
    /// NOTE: port must be non-zero!
    /// NOTE: an IPv4 address of zero means receive from any interface,
    ///       if an address is specified, only receive from an interface with
    ///       that address.
    /// @return
    RetType bind(addr_t& addr) {
        m_addr = addr;
        return m_udp->subscribe_port(this, addr.port);
    }

    /// @brief unbind a socket
    /// @return
    RetType unbind() {
        if(m_addr.port != 0) {
            return m_udp->unsubscribePort(m_addr.port);
        }

        // not bound
        return RET_SUCCESS;
    }

    /// @brief get the Maximum Transmit Unit for this socket
    /// @return the MTU in bytes
    size_t mtu() {
        return MTU;
    }

    /// @brief get how much data can be read without blocking
    /// @return how many packets are available to be read without blocking
    size_t available() {
        return m_rx.size();
    }

    /// @brief send a packet over this socket
    /// @return
    RetType send(uint8_t* buff, size_t len, addr_t* dst) {
        RESUME();

        // payload size must be MTU + FCS
        alloc::Packet<MTU_NO_HEADERS - HEADERS_SIZE, HEADERS_SIZE> packet;

        // push the payload onto the packet
        RetType ret = packet.push(buff, len);

        if(RET_SUCCESS != ret) {
            return ret;
        }

        // fill in information
        netinfo_t info;
        info.ignore_checksums = false;
        info.dst.udp_port = dst->port;
        ipv4::IPv4Address(dst->ip[0], dst->ip[1], dst->ip[2], dst->ip[3],
                                                        &(info.dst.ipv4_addr));

        // first pass transmit
        ret = CALL(m_udp->transmit(packet, info, this));

        if(RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        // reset the headers position for the second pass
        packet.seek_header();

        // second pass transmit
        ret = CALL(m_udp->transmit2(packet, info, this));

        RESET();
        return ret;
    }

    /// @brief receive a packet over this socket
    /// @param buff     the buffer to copy the data into
    /// @param len      pointer to the length of 'buff', must be non-NULL
    /// @param src      a struct to fill source information into, or NULL
    /// @return
    ///
    /// Blocks waiting for a packet to arrive if there is not already one
    /// buffered. Then fills in at most 'len' bytes to 'buff'. 'len' is set
    /// to the actual number of bytes in the packet, whether greater or less
    /// than the size of 'buff'. If 'src' is not NULL, the source address the
    /// packet was sent to is filled in.
    RetType recv(uint8_t* buff, size_t* len, addr_t* src) {
        RESUME();

        // find a packet that's been received
        packet_t** packet_p = m_rx.peek();

        if(packet_p == NULL) {
            // no packets buffered, need to block

            if(m_blocked != -1) {
                // another task is already blocking
                return RET_ERROR;
            }

            // block until a packet arrives
            // 'receive' will wake this task
            m_blocked = sched_dispatched;
            BLOCK();

            // grab the packet that should have just arrived
            // NOTE: assuming this is non-NULL as this task has been woken!
            packet_p = m_rx.peek();
        }

        packet_t* packet = *packet_p;

        // return the packet to the pool and take it's pointer off the queue
        m_rx.pop();

        // NOTE: this should never fail
        if(!m_pool.free(packet)) {
            RESET();
            return RET_ERROR;
        }

        // find the number of bytes to actuall copy to 'buff'
        size_t min = packet->len;
        if(*len < min) {
            min = *len;
        }

        // copy the bytes to buff
        memcpy(buff, packet->buff, min);

        // record the actual length in len
        *len = packet->len;

        // record source information if requested
        if(NULL != src) {
            src->port = packet->port;
            memcpy(src->ip, packet->ip, 4);
        }

        RESET();
        return RET_SUCCESS;
    }

    /// @brief provide a packet to the socket layer
    /// @param packet   the received packet
    /// @param info     information about the packet to be filled in
    /// @param caller   the layer that called this function one layer before
    RetType receive(Packet& packet, netinfo_t& info, NetworkLayer*) {
        size_t size = packet.available();

        // check if this packet is properly sized
        if(size > MTU) {
            // too much data to store
            // NOTE: we could instead of dropping the packet just truncate it
            return RET_ERROR;
        }

        // check if the address is correct
        // NOTE: we can assume the UDP port is ours since it was delivered to us
        if((m_addr.ip[0] | m_addr.ip[1] | m_addr.ip[2] | m_addr.ip[3]) != 0) {
            ipv4::IPv4Addr_t addr;
            ipv4::IPv4Address(m_addr.ip[0], m_addr.ip[1],
                              m_addr.ip[2], m_addr.ip[3], &addr);

            if(addr != info.dst.ipv4_addr) {
                // this packet isn't for us
                return RET_ERROR;
            }
        } // an address of 0 means we accept the packet from any interface

        // try to allocate space to copy the packet
        packet_t* buff = m_pool.alloc();

        // if no buffers are available, drop the oldest buffer
        if(buff == NULL) {
            packet_t** oldest = m_rx.peek();
            m_rx.pop();
            m_pool.free(*oldest);

            buff = m_pool.alloc();
            // NOTE: assuming buff is non-NULL here
        }

        // push the newly allocated buffer onto the received queue
        m_rx.push(buff);

        // copy the payload into the receive buffer
        if(RET_SUCCESS != packet.read(buff->buff, size)) {
            return RET_ERROR;
        }

        buff->len = size;

        // copy addressing info from the source
        buff->port = info.src.udp_port;
        buff->ip[0] = info.src.ipv4_addr << 24;
        buff->ip[1] = info.src.ipv4_addr << 16;
        buff->ip[2] = info.src.ipv4_addr << 8;
        buff->ip[3] = info.src.ipv4_addr;

        // unblock the task waiting for a packet if there is one
        if(m_blocked != -1) {
            WAKE(m_blocked);
        }

        return RET_SUCCESS;
    }

    /// @brief not implemented
    RetType transmit(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        return RET_ERROR;
    }

    /// @brief not implemented
    RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        return RET_ERROR;
    }

protected:
    // packet type for receive buffer
    typedef struct {
        uint8_t buff[MTU];
        size_t len;
        uint8_t ip[4];
        uint16_t port;
    } packet_t;

    /// @brief protected constructor
    IPv4UDPSocket(Queue<packet_t*>& receive_queue,
                  Pool<packet_t>& packet_pool)    : m_rx(receive_queue),
                                                    m_pool(packet_pool),
                                                    m_udp(NULL),
                                                    m_addr({0, 0}),
                                                    m_blocked(-1) {};

private:
    // received packets queue
    Queue<packet_t*>& m_rx;

    // packet pool
    Pool<packet_t>& m_pool;

    // UDP router
    udp::UDPRouter* m_udp;

    // the bound address
    addr_t m_addr;

    // any blocked task
    tid_t m_blocked;
};


namespace alloc {

/// @brief IPv4/UDP stack socket with preallocated receive buffer
/// @param SIZE     how many packets can be buffered before one is discarded
template <size_t SIZE>
class IPv4UDPSocket : public ::IPv4UDPSocket {
public:
    /// @brief constructor
    IPv4UDPSocket() : ::IPv4UDPSocket(m_buff, m_pool) {};

private:
    // allocated packet buffer
    alloc::Queue<::IPv4UDPSocket::packet_t*, SIZE> m_buff;

    // allocated packet pool
    alloc::Pool<::IPv4UDPSocket::packet_t, SIZE> m_pool;
};

} // namespace alloc

#endif
