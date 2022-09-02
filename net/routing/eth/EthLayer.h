#ifndef ETH_LAYER_H
#define ETH_LAYER_H

#include "net/NetworkLayer.h"
#include "net/eth/eth.h"

/// @brief Ethernet (layer 2) layer
class EthLayer : public NetworkLayer {
public:
    /// @brief constructor
    /// @param mac  the MAC address of the device
    // TODO also take in an ARP layer
    EthLayer(uint8_t mac[6]) {
        for(size_t i = 0; i < 6; i++) {
            m_mac[i] = mac[i];
        }
    }

    /// @brief receive a packet
    ///        drops packet if dst is not this device's MAC or a broadcast/multicast
    /// @return
    RetType receive(Packet& packet, msg_t& info, NetworkLayer*) {
        EthHeader_t* hdr = reinterpret_cast<EthHeader_t*>(packet->raw());

        if(hdr->dst[0] & 0b1) {
            // a 1 in the least significant bit of the first octet is a multicast
            return RET_SUCCESS;
        }

        bool match = true;
        bool broadcast = true;
        for(size_t i = 0; i < 6; i++) {
            if(hdr->dst[i] != m_mac[i]) {
                // bad dst MAC
                match = false;
            }

            if(hdr->dst[i] != 0xFF) {
                broadcast = false;
            }
        }

        if(match | broadcast) {
            return RET_SUCCESS;
        }

        // no match
        return RET_ERROR;
    }

    /// @brief transmit a packet
    /// @return
    RetType transmit(Packet& packet, msg_t& msg, NetworkLayer*) {
        // TODO

        return RET_SUCCESS;
    }

private:
    uint8_t m_mac[6];
};

#endif
