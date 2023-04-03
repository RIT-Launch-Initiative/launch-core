/*******************************************************************************
*
*  Name: SimpleArpLayer.h
*
*  Purpose: Subsitutes the ARP protocol for static mapping of IPv4 addresses
*           to MAC addresses. Rather than use ARP to query for a MAC address,
*           the MAC is set to be 6c:69:A:B:C:D, where A, B, C, D and the octets
*           from the IPv4 address being sent to (A.B.C.D).
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef SIMPLE_ARP_H
#define SIMPLE_ARP_H

#include <string.h>

#include "net/network_layer/NetworkLayer.h"
#include "sched/macros.h"
#include "net/ipv4/ipv4.h"

class SimpleArpLayer : public NetworkLayer {
public:
    /// @brief constructor
    /// @param out  the network layer to send transmitting packets too
    SimpleArpLayer(NetworkLayer& lower) : m_lower(lower) {};

    /// @brief receive
    /// @returns error all the time, cannot receive
    RetType receive(Packet& packet, sockinfo_t& info, NetworkLayer*) {
        // TODO could add responding to ARP requests here
        // send response out the same layer that received

        return RET_ERROR;
    }

    /// @brief transmit
    ///        fills in the 'mac' field of info.dst
    /// @return
    RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer*) {
        RESUME();

        if(ipv4::is_multicast(&(info.dst.ipv4_addr)) ||
           ipv4::is_broadcast(&(info.dst.ipv4_addr))) {
               // use the broadcast MAC address
               memset(&(info.dst.mac), 0xFF, 6);
        } else {
            // use the MAC address FIXED1:FIXED2:A:B:C:D
            // where FIXED1 and FIXED2 are fixed bytes and
            // A.B.C.D is the desintation IPv4 address
            info.dst.mac[0] = FIXED_MAC_1;
            info.dst.mac[1] = FIXED_MAC_2;

            uint32_t& ip = info.dst.ipv4_addr;
            info.dst.mac[2] = ip;
            info.dst.mac[3] = ip >> 8;
            info.dst.mac[4] = ip >> 16;
            info.dst.mac[5] = ip >> 24;
        }

        RetType ret = CALL(m_lower.transmit(packet, info, this));

        RESET();
        return ret;
    }

    /// @brief transmit (second pass)
    /// @return
    RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();

        RetType ret = CALL(m_lower.transmit2(packet, info, this));

        RESET();
        return ret;
    }

    static const uint8_t FIXED_MAC_1 = 0x6c;
    static const uint8_t FIXED_MAC_2 = 0x69;

private:
    NetworkLayer& m_lower;
};

#endif
