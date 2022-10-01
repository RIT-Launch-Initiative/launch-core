#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include "../socket/Socket.h"
#include "../packet/Packet.h"
#include "../network_layer/NetworkLayer.h"

typedef struct {
    sockaddr_t src,
    sockaddr_t dst,
    uint32_t length,
    int checksum,
    uint8_t *data_octets
} UDP_HEADER_T;

class UDP : public NetworkLayer {
public:


    UDP(Socket socket) {
    }

    int calc_checksum(uint8_t *payload) {
        int checksum = 0;
        for (int i = 0; i < sizeof(payload); i++) {
            checksum += payload[i];
        }

        return checksum;
    }

    RetType receive(Packet &packet, sockmsg_t &info, NetworkLayer *caller) {
        int checksum = calc_checksum(info.payload);
        UDP_HEADER_T header = packet.allocate_header<UDP_HEADER_T>();



        return RET_SUCCESS;
    }

    RetType transmit(Packet &packet, sockmsg_t &info, NetworkLayer *caller) {
        int checksum = calc_checksum(info.payload);
        UDP_HEADER_T header = packet.allocate_header<UDP_HEADER_T>();


        return RET_SUCCESS;
    }


};


#endif //LAUNCH_CORE_UDP_H
