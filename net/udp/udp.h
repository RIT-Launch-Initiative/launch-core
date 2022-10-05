#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include <stdint.h>


namespace udp {
    typedef struct {
        uint16_t src,
        uint16_t dst,
        uint16_t checksum,
        uint16_t length,
        uint32_t *data_octets
    } UDP_HEADER_T;

    uint16_t calc_checksum(uint8_t *payload) {
        int checksum = 0;
        for (int i = 0; i < sizeof(payload); i++) {
            checksum += payload[i];
        }

        return checksum;
    }

    bool verify_checksum(uint8_t *payload, uint16_t checksum) {
        int result;

        for (int i = 0; i < sizeof(payload); i++) {
            result += payload[i];
        }

        return checksum == result;
    }
}

#endif //LAUNCH_CORE_UDP_H
