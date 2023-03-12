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

#include "net/socket/socket.h"
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


class IPv4UDPSocket : public NetworkLayer {
public:
    /// @brief constructor
    IPv4UDPSocket() {};

    /// @brief set the stack this socket belongs to
    /// @param stack    the stack
    /// @return
    void set_stack(IPv4UDPStack* stack) {
        m_stack = stack;
    }

    /// @brief bind a socket to send/receive from an address
    /// this may mean different things depending on the socket type
    RetType bind(ADDR& addr) {

    }

    /// @brief get the Maximum Transmit Unit for this socket
    size_t mtu() {
        size_t len = 1518; // maximum ethernet frame size, including header

        len -= sizeof(eth::EthHeader_t);
        len -= sizeof(uint16_t); // ethernet FCS
        len -= sizeof(ipv4::IPv4Header_t);
        len -= sizeof(udp::UDP_HEADER_T);

        return len;
    }

    /// @brief get how much data can be read without blocking
    /// @return how many packets are available to be read without blocking
    size_t available() {
        return m_rx.size();
    }

    /// @brief send a packet over this socket
    /// @return
    RetType send(uint8_t* buff, size_t len, ADDR& dst) {

    }

    /// @brief receive a packet over this socket
    /// source address of the packet will be filled into 'src'
    /// @return
    RetType recv(uint8_t* buff, size_t len, ADDR& src) {

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
        // queue the packet up to be read by the user

        // TODO also queue up received from address
    }

    /// @brief send a packet through the stack
    ///        pushes the packet down the stack
    ///        The header position of 'packet' should be at the last allocated header
    ///        The write position should be at the end of the payload
    /// @param packet   the packet to transmit
    /// @param info     information about the packet
    /// @param caller   the layer that called this function one layer before
    /// NOTE: this function should be reentrant and use scheduler macros,
    ///       layers are allowed to block
    ///       this also means this function must be called in a task
    /// @return
    virtual RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer* caller) = 0;

    /// @brief second pass for transmitting a packet
    ///        The header position of 'packet' should be at the last allocated header
    ///        The write position should be at the end of the payload
    virtual RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) = 0;

private:
    // received packets
    // TODO don't hardcode size of this buffer
    alloc::Queue<Packet*, 10> m_rx;

    // the networking stack this socket belongs to
    IPv4UDPStack* m_stack;
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
                                          m_eth({SimpleArpLayer::FIXED_MAC_1,
                                                 SimpleArpLayer::FIXED_MAC_2,
                                                 a, b, c, d},
                                                 m_dev,
                                                 m_arp),
                                                 eth::IPV4_PROTO,
                                                 true),
                                          m_lo(),
                                          m_socks(),
                                          packets() {
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

        ipv4::IPv4Addr(127, 0, 0, 0, &temp_addr);
        ipv4::IPv4Addr(255, 0, 0, 0, &temp_subnet);

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
    Socket* get_socket() {
        IPv4UDPSocket* sock = m_socks.alloc();

        if(sock != NULL) {
            sock->set_stack(this);
        }

        return sock;
    }

    /// @brief free a socket, returning it back to the stack
    /// @return
    void free_socket(Socket* sock) {
        m_socks.free(static_cast<IPv4IPv4UDPSocket*>(sock));
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

    /// @brief subscribe a socket to receive packets on a particular UDP port
    /// @param sock     the socket to subscribe
    /// @param port     the UDP port, in system endianness
    /// @return
    RetType subscribe(IPv4UDPSocket& sock, uint16_t port) {
        return m_udp.subscribe_port(sock, port);
    }

    /// @brief get the top layer of the networking stack
    NetworkLayer& top_layer() {
        return m_udp;
    }

    // pool of packets
    // TODO don't hardcode this!
    alloc::Pool<alloc::Packet<1500, 200>, 32> packets;

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
    ipv4::IPv4Address_t m_ipAddr;
    ipv4::IPv4Address_t m_ipSubnet;

    // pool of sockets
    // TODO don't hardcode this!
    alloc::Pool<IPv4UDPSocket, 32> m_socks;
};

#endif
