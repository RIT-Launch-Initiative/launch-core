#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include "net/socket/Socket.h"
#include "net/packet/Packet.h"
#include "net/network_layer/NetworkLayer.h"
#include <stdint.h>

typedef struct {
    uint8_t src,
    uint8_t dst,
    uint8_t checksum,
    uint32_t length,
    uint8_t *data_octets
} UDP_HEADER_T;

class UDP : public NetworkLayer {
public:


    virtual RetType create_receive_port(uint8_t port_num) = 0;

    virtual RetType receive(Packet &packet, sockmsg_t &info, NetworkLayer *caller) = 0;

    virtual RetType transmit(Packet &packet, sockmsg_t &info, NetworkLayer *caller) = 0;

private:
    uint32_t calc_checksum(uint8_t *payload) {
        int checksum = 0;
        for (int i = 0; i < sizeof(payload); i++) {
            checksum += payload[i];
        }

        return checksum;
    }

    bool verify_checksum(uint8_t *payload, int checksum) {
        int result;

        for (int i = 0; i < sizeof(payload); i++) {
            result += payload[i];
        }

        return checksum == result;
    }


};


#endif //LAUNCH_CORE_UDP_H
