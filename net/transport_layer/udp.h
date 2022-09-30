#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include "socket/Socket.h"
#include "packet/Packet.h"
#include ""

class UDP : public NetworkLayer {
public:
    UDP(Socket socket) {

    }

    RetType receive(Packet &packet, sockmsg_t &info, NetworkLayer *caller) {

        return RET_SUCCESS;
    }

    RetType transmit(Packet &packet, sockmsg_t &info, NetworkLayer *caller) {

        return RET_SUCCESS;
    }


};


#endif //LAUNCH_CORE_UDP_H
