#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include "socket/Socket.h"
#include "packet/Packet.h"
#include ""

class UDP : public NetworkLayer {
public:
    UDP(Socket socket) {

    }

    RetType receive(Packet& packet, sockmsg_t& info, NetworkLayer* caller) {

    }

    RetType transmit(Packet& packet, sockmsg_t& info, NetworkLayer* caller) {

    }


};


#endif //LAUNCH_CORE_UDP_H
