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

    // TODO: Need to pass in headers
    uint16_t checksum(UDP_HEADER_T udp_hdr, const uint16_t* data, uint16_t len) {
        uint16_t sum = 0;

        // TODO: Calculate IP psuedo header


        sum += udp_hdr.src;
        sum += udp_hdr.dst;
        sum += udp_hdr.checksum;
        sum += udp_hdr.length;



        for(int i = 0; i < (len / sizeof(uint16_t)); i++) {
            sum += data[i];
        }

        return ~sum;
    }


    bool verify_checksum(UDP_HEADER_T header, uint16_t *payload, uint16_t actual_checksum) {
        // TODO: Update size
        return checksum(header, payload, sizeof(UDP_HEADER_T) + sizeof(*payload)) == actual_checksum;
    }
}

#endif //LAUNCH_CORE_UDP_H
