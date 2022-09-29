#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include "../socket/Socket.h"
#include "../packet/Packet.h"

class UDP {
public:
    Socket socket; // TODO: Is raw necessary?

    UDP(sockaddr_t* addr) {
        socket = Socket().init;
    }

    RetType send_packet(uint8_t* data) {
        int packet_size = 31 + sizeof(data) + 1;
        const int header_size = 8;
        Packet packet = alloc::Packet<const_cast<int>(header_size), header_size>();
        packet.push(data);

        return RET_SUCCESS;
    }


};


#endif //LAUNCH_CORE_UDP_H
