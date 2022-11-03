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
    

    virtual RetType recieve(Packet& packet, sockmsg_t& info, NetworkLayer* caller) {
        // Process echo request
        
    }

    virtual RetType transmit(Packet& packet, sockmsg_t& info, NetworkLayer* caller) {
        return(RET_ERROR);
        
    }

    // function to allow caller to be associated with an ip
    virtual sockaddr_t ipconvert(NetworkLayer *caller) {
        
    }
};


