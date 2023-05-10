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
        // TODO define constants for packets
    } icmphead_t

    virtual RetType recieve(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        icmphead_t rec = packet read  // Do something like this that actually works
                                      // Check if the packet address matches the destination
                                      // if so, send packet to caller
                                      // if not, packet is junk
    }

    virtual RetType transmit(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        return(RET_ERROR);     
    }

    virtual RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        return(RET_ERROR);
    }
        
};


