#ifndef ARP_LAYER_H
#define ARP_LAYER_H

#include "net/network_layer/NetworkLayer.h"
#include "net/eth/EthLayer.h"
#include "net/ipv4/ipv4.h"
#include "sched/macros.h"

class ArpLayer : public NetworkLayer {
public:
    /// @brief constructor
    /// @param addr   this devices IPv4 address
    /// @param eth    ethernet layer
    ///               packets should be received from this device
    ///               requests and replies will be sent from this device
    ArpLayer(IPv4Addr_t& addr, EthLayer& eth) : m_addr(addr), m_eth(eth) {};



    // TODO
    // should be able to block and wake tasks waiting for ARP response

    /// @brief lookup a MAC address in the ARP table
    /// @param addr     address to lookup
    /// @param mac      where the MAC will be copied to
    /// @return
    /// NOTE: this function may block in order to query
    RetType lookup(IPv4Addr_t& addr, uint8_t mac[6]) {
        // TODO

        return RET_BLOCKED;
    }

private:
    IPv4Addr_t& m_addr;
    EthLayer& m_eth;


    // TODO keep a table of queues of tid's waiting for addresses
};

#endif
