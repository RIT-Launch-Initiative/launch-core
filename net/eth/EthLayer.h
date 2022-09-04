#ifndef ETH_LAYER_H
#define ETH_LAYER_H

#include "net/network_layer/NetworkLayer.h"
#include "net/eth/eth.h"
#include "sched/macros.h"

/// @brief Ethernet (layer 2) layer
class EthLayer : public NetworkLayer {
public:
    /// @brief constructor
    /// @param mac      the MAC address of the device
    /// @param in       the network layer to forward received packets to
    /// @param out      the network layer to transmit packets to
    // TODO also take in an ARP layer
    EthLayer(uint8_t mac[6], NetworkLayer& in, NetworkLayer& out) : m_in(in), m_out(out) {
        for(size_t i = 0; i < 6; i++) {
            m_mac[i] = mac[i];
        }
    }

    /// @brief receive a packet
    ///        drops packet if dst is not this device's MAC or a broadcast/multicast
    /// @return
    RetType receive(Packet& packet, msg_t& info, NetworkLayer*) {
        RESUME();

        EthHeader_t* hdr = packet.ptr<EthHeader_t>();

        if(hdr == NULL) {
            return RET_ERROR;
        }

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
                // not all 1s
                broadcast = false;
            }
        }

        RetType ret = RET_ERROR;

        if(match | broadcast) {
            if(RET_SUCCESS == packet.skip_read(sizeof(EthHeader_t))) {
                ret = CALL(m_in.receive(packet, info, this));
            }
        }

        RESET();
        return ret;
    }

    /// @brief transmit a packet
    /// @return
    RetType transmit(Packet& packet, msg_t& msg, NetworkLayer*) {
        RESUME();

        if(RET_SUCCESS != packet.reverse(sizeof(EthHeader_t))) {
            return RET_ERROR;
        }

        EthHeader_t* hdr = packet.ptr<EthHeader_t>();

        if(hdr == NULL) {
            return RET_ERROR;
        }

        for(size_t i = 0; i < 6; i++) {
            hdr.src[i] = m_mac[i];
        }

        // TODO ARP lookup for dst

        // TODO pass in the protocol somehow (IPv4)

        RetType ret = CALL(m_out.transmit(packet, msg, this));

        RESET();
        return ret;
    }

private:
    uint8_t m_mac[6];

    NetworkLayer& m_in;
    NetworkLayer& m_out;
};

#endif
