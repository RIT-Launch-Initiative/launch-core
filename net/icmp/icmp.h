#include "net/network_layer/NetworkLayer.h"

class NetworkLayer {
public:
    

    virtual RetType recieve(Packet& packet, sockmsg_t& info, NetworkLayer* caller) {
    
        
    }

    virtual RetType transmit(Packet& packet, sockmsg_t& info, NetworkLayer* caller) {
    return(RET_ERROR);
        
    }
};


