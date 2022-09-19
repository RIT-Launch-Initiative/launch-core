#ifndef ARP_LAYER_H
#define ARP_LAYER_H

#include "net/network_layer/NetworkLayer.h"
#include "net/eth/eth.h"
#include "net/ipv4/IPv4Router.h"
#include "sched/macros.h"

using ipv4::IPv4Router;
using ipv4::IPv4Addr_t;

/// @brief implements the ARP layer
///        Requires an IPv4 router.
///
///      receiving:
///        Received ARP packets will be parsed and the IPv4 address will be
///        looked up in the IPv4 router. For ARP requests, if the device that
///        the address belongs too is the address that received the packet, a
///        response is sent with the devices MAC address. MAC addresses of
///        devices need to be manually added using the "add_dev" function.
///
///      lookup:
///        MAC addresses corresponding to an IPv4 address can be looked up. If
///        the address is already known it is returned immediately, if it is not
///        and ARP request is sent. This layer will handle blocking the calling
///        task until a response is received (or timeout).
///
///      transmit:
///        Transmits are invalid for the ARP layer.
class ArpLayer : public NetworkLayer {
public:
    /// @brief constructor
    /// @param router       the IPv4 router for the network stack
    ArpLayer(IPv4Router& router) : m_router(router) {};


    /// @brief add an Ethernet device to the ARP table
    ///        this tells the ARP table the MAC address of this device so valid
    ///        replies can be sent
    /// @param dev  the device
    /// @param mac  the mac address of the device
    /// @return
    RetType add_dev(NetworkLayer& eth_dev, uint8_t mac[6]) {
        mac_t addr;
        addr.addr[0] = mac[0];
        addr.addr[1] = mac[1];
        addr.addr[2] = mac[2];
        addr.addr[3] = mac[3];
        addr.addr[4] = mac[4];
        addr.addr[5] = mac[5];

        NetworkLayer** ptr = m_devs.add(addr)

        if(ptr == NULL) {
            // no room
            return RET_ERROR;
        }

        // set the device
        *ptr = &eth_dev;

        return RET_SUCCESS;
    }

    /// @brief lookup a MAC address in the ARP table
    /// @param addr     address to lookup
    /// @param mac      where the MAC will be copied to
    /// @return
    /// NOTE: this function may block in order to query
    RetType lookup(IPv4Addr_t& addr, uint8_t mac[6]) {
        // TODO

        return RET_ERROR;
    }

    /// @brief receive an ARP packet
    /// @return
    RetType receive(Packet& packet, sockmsg_t& info, NetworkLayer* caller) {
        // TODO

        return RET_ERROR;
    }

    /// @brief transmit
    /// @return error, always invalid
    RetType transmit(Packet&, sockmsg_t&, NetworkLayer*) {
        return RET_ERROR;
    }

private:
    IPv4Router& m_router;

    typedef struct {
        uint8_t addr[6];
    } mac_t;

    // TODO hardcoding size!
    // maps IPv4 address to MAC addresses
    alloc::Hashmap<IPv4Addr_t, mac_t, 25, 25> m_table;

    // TODO hardcoding size!
    // maps MAC addresses to devices
    alloc::Hashmap<mac_t, NetworkLayer*, 25, 25> m_devs;
};

#endif
