#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include <stdint.h>
#include "net/socket/Socket.h"
#include "net/ipv4/ipv4.h"

namespace udp {
    typedef struct {
        uint16_t length;
        uint16_t checksum;
        uint16_t dst;
        uint16_t src;
    } UDP_HEADER_T;

    uint16_t checksum(ipv4::IPv4Addr_t src, ipv4::IPv4Addr_t dst, const uint16_t *data, uint16_t len) {
        uint16_t sum = 0;

        sum += src;
        sum += dst;
        sum += static_cast<uint8_t>(0x11);
        sum += len;

        for (int i = 0; i < len; i++) {
            sum += data[i];
        }

        return ~sum;
    }
}

#endif //LAUNCH_CORE_UDP_H
