/*******************************************************************************
*
*  Name: IPv4UDPStack.h
*
*  Purpose: Implements an IPv4/UDP/Ethernet II networking stack
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/

#ifndef IPV4_UDP_STACK_H
#define IPV4_UDP_STACK_H

#include <stdint.h>

// #include "net/socket/Socket.h"
#include "net/udp/udp.h"
#include "net/udp/UDPRouter.h"
#include "net/ipv4/ipv4.h"
#include "net/ipv4/IPv4Router.h"
#include "net/eth/eth.h"
#include "net/eth/EthLayer.h"
#include "net/loopback/Loopback.h"
#include "net/simple_arp/SimpleArpLayer.h"

#include "return.h"
#include "net/network_layer/NetworkLayer.h"
#include "queue/allocated_queue.h"
#include "net/packet/Packet.h"
#include "pool/pool.h"
#include "sched/macros.h"


// TODO this is pretty messy, need to clean it up

/// @brief address type
typedef struct {
    uint8_t ip[4];
    uint16_t port;
} udp_ip4_addr_t;

class IPv4UDPSocket : public NetworkLayer {
public:
    /// @brief constant MTU
    static const size_t MTU = 1518 - sizeof(eth::EthHeader_t) \
                                   - sizeof(uint16_t) \
                                   - sizeof(ipv4::IPv4Header_t) \
                                   - sizeof(udp::UDP_HEADER_T);

    /// @brief constructor
    IPv4UDPSocket() : m_port(0), m_blocked(-1), m_udp(NULL) {};

    /// @brief set the UDP layer to interact with
    /// @return
    void set_udp(udp::UDPRouter* udp) {
        m_udp = udp;
    }

    /// @brief bind a socket to send/receive from a port
    /// @return
    RetType bind(udp_ip4_addr_t& addr) {
        m_port = addr.port;
        return m_udp->subscribe_port(this, addr.port);
    }

    /// @brief unbind a socket
    /// @return
    RetType unbind() {
        if(m_port) {
            return m_udp->unsubscribePort(m_port);
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
    RetType send(uint8_t* buff, size_t len, udp_ip4_addr_t& dst) {
        RESUME();

        alloc::Packet<MTU, 1518 - MTU> packet;

        RetType ret = packet.push(buff, len);

        if(RET_SUCCESS != ret) {
            return ret;
        }

        sockinfo_t info;
        info.dst.udp_port = dst.port;
        ipv4::IPv4Address(dst.ip[0], dst.ip[1], dst.ip[2], dst.ip[3], &(info.dst.ipv4_addr));

        // first pass transmit
        ret = CALL(m_udp->transmit(packet, info, this));

        if(RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        packet.seek_header();

        // second pass transmit
        ret = CALL(m_udp->transmit2(packet, info, this));

        RESET();
        return ret;
    }

    /// @brief receive a packet over this socket
    /// source address of the packet will be filled into 'src'
    /// @return
    /// NOTE: blocks until a packet is available to receive
    /// reads at most 'len' bytes of a packet into 'buff'
    /// if the actual packet is longer, the remaining bytes are truncated and discarded
    RetType recv(uint8_t* buff, size_t len, udp_ip4_addr_t& src) {
        RESUME();

        packet_t* packet = m_rx.peek();

        if(packet == NULL) {
            // need to block
            if(m_blocked == -1) {
                // another task is already blocking
                return RET_ERROR;
            }

            // block until a packet arrives
            // 'receive' will wake this task
            m_blocked = sched_dispatched;
            BLOCK();

            packet = m_rx.peek();
        }

        size_t min = packet->len;
        if(len < min) {
            min = len;
        }

        memcpy(buff, packet->buff, min);
        src.port = packet->port;

        // TODO set IP as well

        RESET();
        return RET_SUCCESS;
    }

    /// @brief provide the layer with a received packet
    ///        pushes the packet up the stack
    /// @param packet   the received packet
    /// @param info     information about the packet to be filled in
    /// @param caller   the layer that called this function one layer before
    /// NOTE: this function should be reentrant and use scheduler macros,
    ///       layers are allowed to block
    ///       this also means this function must be called in a task
    /// @return
    RetType receive(Packet& packet, sockinfo_t& info, NetworkLayer*) {
        packet_t* buff = m_rx.push();

        // if no buffers are available, drop the oldest buffer
        if(buff == NULL) {
            m_rx.pop();
            buff = m_rx.push();
        }

        size_t size = packet.available();

        if(size > MTU) {
            // too much data to read
            return RET_ERROR;
        }

        if(RET_SUCCESS != packet.read(buff->buff, size)) {
            return RET_ERROR;
        }

        buff->len = size;
        buff->port = info.src.udp_port;

        // unblock the task waiting for a packet if there is one
        if(m_blocked != -1) {
            WAKE(m_blocked);
        }

        return RET_SUCCESS;
    }

    /// @brief not implemented
    RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        return RET_ERROR;
    }

    /// @brief not implemented
    RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        return RET_ERROR;
    }

private:
    typedef struct {
        uint8_t buff[MTU];
        size_t len;
        uint16_t port;
    } packet_t;

    // received packets
    // TODO don't hardcode size of this buffer
    alloc::Queue<packet_t, 10> m_rx;

    // UDP layer
    udp::UDPRouter* m_udp;

    // the bound UDP port
    uint16_t m_port;

    // any blocked task
    tid_t m_blocked;
};

class IPv4UDPStack {
public:
    /// @brief constructor
    /// @param a,b,c,d   the IPv4 address of the device a.b.c.d
    /// @param e,f,g,h   the subnet of the device e.f.g.h
    /// @param dev       the physical Ethernet device to deliver packets to
    /// @return
    IPv4UDPStack(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                 uint8_t e, uint8_t f, uint8_t g, uint8_t h,
                 NetworkLayer& dev)
                                        : m_dev(dev),
                                          m_udp(m_ip),
                                          m_ip(),
                                          m_arp(m_eth),
                                          m_eth(SimpleArpLayer::FIXED_MAC_1,
                                                SimpleArpLayer::FIXED_MAC_2,
                                                a, b, c, d,
                                                m_dev,
                                                m_arp,
                                                eth::IPV4_PROTO,
                                                true),
                                          m_lo(),
                                          m_socks() {
        ipv4::IPv4Address(a, b, c, d, &m_ipAddr);
        ipv4::IPv4Address(e, f, g, h, &m_ipSubnet);
    };

    /// @brief initialize the stack
    /// @return
    RetType init() {
        RetType ret;

        // add a route to localhost
        ipv4::IPv4Addr_t temp_addr;
        ipv4::IPv4Addr_t temp_subnet;

        ipv4::IPv4Address(127, 0, 0, 1, &temp_addr);
        ipv4::IPv4Address(255, 0, 0, 0, &temp_subnet);

        ret = m_ip.add_route(temp_addr, temp_subnet, m_lo);

        if(RET_SUCCESS != ret) {
            return ret;
        }

        // add a route for the device
        ret = m_ip.add_route(m_ipAddr, m_ipSubnet, m_dev);

        if(RET_SUCCESS != ret) {
            return ret;
        }

        // add the UDP router as a protocol handler
        ret = m_ip.add_protocol(ipv4::UDP_PROTO, m_udp);

        return ret;
    }

    /// @brief get a socket from the stack
    /// @return a ponter to the socket, or NULL on error
    IPv4UDPSocket* get_socket() {
        IPv4UDPSocket* sock = m_socks.alloc();

        if(sock != NULL) {
            sock->set_udp(&m_udp);
        }

        return sock;
    }

    /// @brief free a socket, returning it back to the stack
    /// @return
    void free_socket(IPv4UDPSocket* sock) {
        sock->unbind();
        m_socks.free(sock);
    }

    /// @brief add a multicast address for the stack to listen for
    /// @return
    RetType add_multicast(ipv4::IPv4Addr_t addr) {
        // adds addr/32 as a route for the ethernet layer
        // all packets sent to 'addr' over 'm_eth' will have a longest prefix
        // match with addr/32 (since it's the longest match possible)
        return m_ip.add_route(addr, 0xFFFFFFFF, m_eth);
    }

    /// @brief remove a multicast address for the stack to listen to
    /// @return
    RetType remove_multicast(ipv4::IPv4Addr_t addr) {
        return m_ip.remove_route(addr);
    }

private:
    // UDP Router
    udp::UDPRouter m_udp;

    // IPv4 Router
    ipv4::IPv4Router m_ip;

    // (simple) ARP layer
    SimpleArpLayer m_arp;

    // Ethernet layer
    EthLayer m_eth;

    // Loopback device
    Loopback m_lo;

    // Device layer
    NetworkLayer& m_dev;

    // IP address and subnet of the device
    ipv4::IPv4Addr_t m_ipAddr;
    ipv4::IPv4Addr_t m_ipSubnet;

    // pool of sockets
    // TODO don't hardcode this!
    alloc::Pool<IPv4UDPSocket, 32> m_socks;
};

#endif
