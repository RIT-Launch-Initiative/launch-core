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
#include "net/stack/IPv4UDP/IPv4UDPSocket.h"



// TODO make this store multiple Ethernet devices? (or any layer 1 device)
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
                                                dev,
                                                m_ip,
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

        ret = m_ip.add_outgoing_route(temp_addr, temp_subnet, m_lo);

        if(RET_SUCCESS != ret) {
            return ret;
        }

        ret = m_ip.add_incoming_route(temp_addr, m_lo);

        if(RET_SUCCESS != ret) {
            return ret;
        }

        // add a route for packets bound for the device
        // outgoing packets go to the ARP layer first
        ret = m_ip.add_outgoing_route(m_ipAddr, m_ipSubnet, m_arp);

        if(RET_SUCCESS != ret) {
            return ret;
        }

        // incoming packets come straight from the Ethernet layer
        ret = m_ip.add_incoming_route(m_ipAddr, m_eth);

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
        m_socks.free(static_cast<alloc::IPv4UDPSocket<10>*>(sock));
    }

    /// @brief add a multicast address for the stack to listen for
    /// @return
    RetType add_multicast(ipv4::IPv4Addr_t addr) {
        // adds addr/32 as a route for the ethernet layer / ARP layer
        // all packets sent to 'addr' will have a longest prefix
        // match with addr/32 (since it's the longest match possible)
        RetType ret;

        ret = m_ip.add_outgoing_route(addr, 0xFFFFFFFF, m_arp);
        if(RET_SUCCESS != ret) {
            return ret;
        }

        ret = m_ip.add_incoming_route(addr, m_eth);
        return ret;
    }

    /// @brief remove a multicast address for the stack to listen to
    /// @return
    RetType remove_multicast(ipv4::IPv4Addr_t addr) {
        m_ip.remove_incoming_route(addr);
        return m_ip.remove_outgoing_route(addr, 0xFFFFFFFF);
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
    alloc::Pool<alloc::IPv4UDPSocket<10>, 4> m_socks;
};

#endif
