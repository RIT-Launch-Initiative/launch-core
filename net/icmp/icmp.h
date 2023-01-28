/*******************************************************************************
* 
*  Name: icmp.h
* 
*  Purpose: Process echo requests and implement ping
* 
*  Author: Chloe Clark
* 
*  RIT Launch Initiative
* 
********************************************************************************/
#ifndef ICMP_H
#define ICMP_H

#include "net/network_layer/NetworkLayer.h"
#include "sched/macros.h"

class ICMP {
public:

    /// @breif constructor
    /// @param out  the network layer to send packets to
    ICMP(NetworkLayer& out) : m_out(out) {};

    // ICMP header for packets
   // typedef struct {
   //     uint8_t* buff;
   //     size_t size;
   //     size_t headerSize;
        // TODO define constants for packets
   // } icmphead_t;

    /// @breif recieve  Get an echo request
    /// @return
    RetType recieve(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();

        //icmphead_t srcPacket;
        //srcPacket.buff = packet.buff;
        //srcPacket->size = packet->size;
        //srcPacket->headerSize = packet->headerSize;
        uint32_t& src = info.src.ipv4_addr;
        uint32_t& dst = info.dst.ipv4_addr;

        if (dst == src) {
            RetType ret = CALL(m_out.transmit(packet, info, caller));       
            RESET();
            return ret;
        }
    }

    /// @breif transmit the packet, echo reply
    /// @return
    RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();
        
        RetType ret = CALL(m_out.transmit(packet, info, caller));

        RESET();
        return ret;
    }

    RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        return RET_ERROR;
    }
        
private:
    NetworkLayer& m_out;
};

#endif
