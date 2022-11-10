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

    typedef struct {
        uint32_t src_addr;
        uint32_t dst_addr;
        uint8_t zero;
        uint8_t protocol;
        uint16_t length;
    } PSEUDO_HEADER_T;

    uint16_t checksum(PSEUDO_HEADER_T *header) {
        const uint16_t *buf = reinterpret_cast<const uint16_t *>(header);
        uint32_t ip_src = header->src_addr;
        uint32_t ip_dst = header->dst_addr;
        uint32_t sum;
        size_t len = header->length;

        sum = 0;
        while (len > 1) {
            sum += *buf++;
            if (sum & 0x80000000) {
                sum = (sum & 0xFFFF) + (sum >> 16);
            }
            len -= 2;
        }

        if (len & 1) {
            sum += *((uint8_t *) buf);
        }

        sum += ip_src++;
        sum += ip_src;

        sum += ip_dst++;
        sum += ip_dst;

        sum += hton16(header->protocol); // 17 is IPPROTO_UDP in Linux
        sum += hton16(header->length);

        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        return (uint16_t) ~sum;
    }
}

#endif //LAUNCH_CORE_UDP_H
