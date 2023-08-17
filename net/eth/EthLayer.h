#ifndef ETH_LAYER_H
#define ETH_LAYER_H

#include "net/network_layer/NetworkLayer.h"
#include "net/eth/eth.h"
#include "sched/macros.h"

using namespace eth;

/// @brief Ethernet (layer 2) layer
class EthLayer : public NetworkLayer {
public:
    /// @brief constructor
    /// @param mac_X    the MAC address of the device a:b:c:d:e:f
    /// @param lower    the network layer outgoing packets should be forwaded to
    ///                 and incoming packets come from
    /// @param upper    the network layer outgoing packets come from and
    ///                 incoming packets should be forwarded to
    /// @param protocol the protocol of packets passed from 'in'
    /// @param add_fcs  true if the FCS should be calculated and added to
    ///                 outgoing packets
    EthLayer(uint8_t mac_a, uint8_t mac_b, uint8_t mac_c,
             uint8_t mac_d, uint8_t mac_e, uint8_t mac_f,
             NetworkLayer& lower,
             NetworkLayer& upper,
             uint16_t protocol,
             bool add_fcs = false) : m_lower(lower), m_upper(upper), m_fcs(add_fcs) {

        m_mac[0] = mac_a;
        m_mac[1] = mac_b;
        m_mac[2] = mac_c;
        m_mac[3] = mac_d;
        m_mac[4] = mac_e;
        m_mac[5] = mac_f;

        m_proto = hton16(protocol);
    }

    /// @brief receive a packet
    ///        drops packet if dst is not this layers MAC or a broadcast/multicast
    /// @return
    RetType receive(Packet& packet, netinfo_t& info, NetworkLayer*) {
        RESUME();

        EthHeader_t* hdr = packet.read_ptr<EthHeader_t>();

        if(hdr == NULL) {
            RESET();
            return RET_ERROR;
        }

        bool match = true;
        if(!(hdr->dst[0] & 0b1)) {
            for(size_t i = 0; i < 6; i++) {
                if(hdr->dst[i] != m_mac[i]) {
                    // bad dst MAC
                    match = false;
                }
            }
        }
        // otherwise a multicast or broadcast address
        // a 1 in the least significant bit of the first octet is a multicast
        // or it's all 1s and is a broadcast


        if(!match) {
            RESET();
            return RET_ERROR;
        }

        // calculate the FCS
        uint32_t calc_fcs = calculate_fcs(packet.raw(), packet.size() + packet.header_size());

        // get the transmitted FCS
        uint32_t fcs = packet.raw()[packet.available() - sizeof(uint32_t)];

        // check that the calculated and sent FCS match
        if(calc_fcs != fcs) {
            // some error occurred in transmission!
            RESET();
            return RET_ERROR;
        }

        // truncate the packet so the FCS isn't included in the payload
        packet.truncate(packet.available() - sizeof(uint32_t));

        // fill in src information for this packet
        for(size_t i = 0; i < 6; i++) {
            info.src.mac[i] = hdr->src[i];
        }

        // skip ahead reading
        if(RET_SUCCESS != packet.skip_read(sizeof(EthHeader_t))) {
            RESET();
            return RET_ERROR;
        }

        // pass the packet to the next layer
        RetType ret = CALL(m_upper.receive(packet, info, this));

        RESET();
        return ret;
    }

    /// @brief transmit a packet
    /// @return
    RetType transmit(Packet& packet, netinfo_t& info, NetworkLayer*) {
        RESUME();

        EthHeader_t* hdr = packet.allocate_header<EthHeader_t>();
        if(hdr == NULL) {
            RESET();
            return RET_ERROR;
        }

        for(size_t i = 0; i < 6; i++) {
            hdr->src[i] = m_mac[i];
            hdr->dst[i] = info.dst.mac[i];
        }

        hdr->ethertype = m_proto;

        RetType ret = CALL(m_lower.transmit(packet, info, this));

        RESET();
        return ret;
    }

    RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer*) {
        RESUME();

        EthHeader_t* hdr = packet.allocate_header<EthHeader_t>();
        if(hdr == NULL) {
            RESET();
            return RET_ERROR;
        }

        // add padding if needed
        ssize_t diff = (packet.size() + packet.header_size() - sizeof(eth::EthHeader_t)) \
                       - eth::MIN_PAYLOAD_SIZE;
        if(diff < 0) {
            // add padding
            uint8_t zero = 0;
            while(diff < 0) {
                packet.push<uint8_t>(zero);
                diff++;
            }
        }

        // calculate the FCS if configured to
        if(m_fcs) {
            uint32_t fcs = calculate_fcs(packet.raw(), packet.size() + packet.header_size());

            if(RET_SUCCESS != packet.push(fcs)) {
                RESET();
                return RET_ERROR;
            }
        }

        // pass the packet along
        RetType ret = CALL(m_lower.transmit2(packet, info, this));

        RESET();
        return ret;
    }

private:
    uint8_t m_mac[6];

    NetworkLayer& m_lower;
    NetworkLayer& m_upper;

    uint16_t m_proto;

    bool m_fcs;
};

#endif
