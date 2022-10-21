#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include <stdint.h>
#include "net/socket/Socket.h"

namespace udp {
    typedef struct {
        uint16_t length;
        uint16_t checksum;
        uint16_t dst;
        uint16_t src;
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
