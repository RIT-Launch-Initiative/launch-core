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
#include "net/network_layer/NetworkLayer.h"

class NetworkLayer {
public:
    
    // ICMP header for packets
    typedef struct {
        uint8_t* buff;
        size_t size;
        size_t headerSize;
        // TODO define constants for packets
    } icmphead_t

    virtual RetType recieve(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        uint32_t src = info->src->ipv4_addr;
        uint32_t dst = info->dst->ipv4_addr;

        icmphead_t rec;
        rec->buff = packet->buff;
        rec->size = packet->size;
        rec->headerSize = packet->headerSize;
        
        if (src == dst) {
            
        } else {
            return(RET_ERROR);
        }
        

                                      // Do something like this that actually works
                                      // Check if the packet address matches the destination
                                      // if so, send packet to caller
                                      // if not, packet is junk
    }

    virtual RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        return(RET_ERROR);     
    }

    virtual RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        return(RET_ERROR);
    }
        
};


