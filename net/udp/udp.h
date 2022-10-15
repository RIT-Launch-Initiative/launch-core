#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include <stdint.h>
#include "net/socket/Socket.h"

namespace udp {
    typedef struct {
        uint32_t *data_octets;
        uint16_t length;
        uint16_t checksum;
        sockport_t dst;
        sockport_t src;
    } UDP_HEADER_T;

    uint16_t calc_checksum(uint8_t *payload) {
        // TODO: Psuedo header issues

        return 0;
    }

    bool verify_checksum(uint8_t *payload, uint16_t checksum) {
        // TODO: Once we figure out pseudo header issue with net stack
        return 0;
    }
}

#endif //LAUNCH_CORE_UDP_H
