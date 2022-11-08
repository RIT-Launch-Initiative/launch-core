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

#include "net/network_layer/NetworkLayer.h"
#include "sched/macros.h"

class SimpleArpLayer : public NetworkLayer {
public:
    /// @brief constructor
    /// @param out  the network layer to send packets to next
    SimpleArpLayer(NetworkLayer& out) : m_out(out) {};

    /// @brief receive
    /// @returns error all the time, cannot receive
    RetType receive(Packet& packet, sockinfo_t& info, NetworkLayer*) {
        return RET_ERROR;
    }

    /// @brief transmit
    ///        fills in the 'mac' field of info.dst
    /// @return
    RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer*) {
        RESUME();

        info.dst.mac[0] = 0x6c;
        info.dst.mac[1] = 0x69;

        uint32_t& ip = info.src.ipv4_addr;
        info.dst.mac[2] = ip;
        info.dst.mac[3] = ip >> 8;
        info.dst.mac[4] = ip >> 16;
        info.dst.mac[5] = ip >> 24;

        RetType ret = CALL(m_out.transmit(packet, info, this));

        RESET();
        return ret;
    }

    /// @brief transmit (second pass)
    /// @return
    RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();

        RetType ret = CALL(m_out.transmit2(packet, info, this));

        RESET();
        return ret;
    }

private:
    NetworkLayer& m_out;
};

#endif
