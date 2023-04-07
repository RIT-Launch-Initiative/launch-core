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
        uint16_t length;
        uint16_t checksum;
    } UDP_HEADER_T;

    // typedef struct {
    //     uint32_t src_addr;
    //     uint32_t dst_addr;
    //     uint8_t zero;
    //     uint8_t protocol;
    //     uint16_t length;
    // } PSEUDO_HEADER_T;

    // uint16_t checksum(PSEUDO_HEADER_T *psuedo_header, UDP_HEADER_T *udp_header) {
    //     udp_header->checksum = 0;
    //     const uint16_t *buf = reinterpret_cast<const uint16_t *>(udp_header);
    //     uint32_t ip_src = hton32(psuedo_header->src_addr);
    //     uint32_t ip_dst = hton32(psuedo_header->dst_addr);
    //     uint32_t sum = 0;
    //     size_t len = hton16(psuedo_header->length);
    //
    //     while (len > 1) {
    //         sum += *buf++;
    //         if (sum & 0x80000000) {
    //             sum = (sum & 0xFFFF) + (sum >> 16);
    //         }
    //         len -= sizeof(uint16_t);
    //     }
    //
    //     if (len & 1) {
    //         sum += *((uint8_t *) buf);
    //     }
    //
    //     sum += ip_src++;
    //     sum += ip_src;
    //
    //     sum += ip_dst++;
    //     sum += ip_dst;
    //
    //
    //     sum += hton16(psuedo_header->protocol); // 17 is IPPROTO_UDP in Linux
    //     sum += hton16(psuedo_header->length);
    //
    //     while (sum >> 16) {
    //         sum = (sum & 0xFFFF) + (sum >> 16);
    //     }
    //
    //     return static_cast<uint16_t>(~sum);
    // }

    uint16_t checksum(UDP_HEADER_T* header, uint8_t* src_ip, uint8_t* dst_ip,
                                         uint8_t* payload, size_t payload_len) {
        // I stole this bad larry from my sys prog project
        uint32_t sum = 0;

        // add payload
        for(size_t i = 0; i < payload_len; i++) {
            if(i & 1) { // odd, low byte
                sum += (uint32_t)payload[i];
            } else { // even, high byte
                sum += (uint32_t)payload[i] << 8;
            }
        }

        // add source IP address (part of the pseudo header)
        for(int i = 0; i < 4; i++) {
            if(i & 1) { // odd, low byte
                sum += (uint32_t)src_ip[i];
            } else { // even, high byte
                sum += (uint32_t)src_ip[i] << 8;
            }
        }

        // add destination IP address (part of the pseudo header)
        for(int i = 0; i < 4; i++) {
            if(i & 1) { // odd, low byte
                sum += (uint32_t)dst_ip[i];
            } else { // even, high byte
                sum += (uint32_t)dst_ip[i] << 8;
            }
        }

        // add the UDP header itself
        uint8_t* data = (uint8_t*)header;
        for(int i = 0; i < sizeof(UDP_HEADER_T); i++) {
            if(i & 1) { // odd, low byte
                sum += (uint32_t)data[i];
            } else { // even, high byte
                sum += (uint32_t)data[i] << 8;
            }
        }

        sum += ipv4::UDP_PROTO;
        sum += ntoh16(header->length);

        while(sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        return hton16(~sum);
    }
}

#endif //LAUNCH_CORE_UDP_H
