#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include <stdint.h>
#include "net/socket/Socket.h"
#include "net/ipv4/ipv4.h"
#include "net/common.h"

namespace udp {
    typedef struct {
        uint16_t src;
        uint16_t dst;
        uint16_t checksum;
        uint16_t length;
    } UDP_HEADER_T;

    typedef struct {
        uint32_t src_addr;
        uint32_t dst_addr;
        uint8_t zero;
        uint8_t protocol;
        uint16_t length;
    } PSEUDO_HEADER_T;

    uint16_t checksum(PSEUDO_HEADER_T *psuedo_header, UDP_HEADER_T *udp_header) {
        udp_header->checksum = 0;
        const uint16_t *buf = reinterpret_cast<const uint16_t *>(udp_header);
        uint32_t ip_src = hton32(psuedo_header->src_addr);
        uint32_t ip_dst = hton32(psuedo_header->dst_addr);
        uint32_t sum = 0;
        size_t len = hton16(psuedo_header->length);

        while (len > 1) {
            sum += *buf++;
            if (sum & 0x80000000) {
                sum = (sum & 0xFFFF) + (sum >> 16);
            }
            len -= sizeof(uint16_t);
        }

        if (len & 1) {
            sum += *((uint8_t *) buf);
        }

        sum += ip_src++;
        sum += ip_src;

        sum += ip_dst++;
        sum += ip_dst;


        sum += hton16(psuedo_header->protocol); // 17 is IPPROTO_UDP in Linux
        sum += hton16(psuedo_header->length);

        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        return static_cast<uint16_t>(~sum);
    }
}

#endif //LAUNCH_CORE_UDP_H
