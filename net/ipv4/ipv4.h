#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>

#include "net/socket/Socket.h"

namespace ipv4 {

static const int DEFAULT_VERSION_IHL = 0x45; // version 4, length 5 (4 * 5 = 20 bytes)
static const int DEFAULT_TTL = 255;

// protocol numbers
static const uint8_t UDP_PROTO          = 0x11;
static const uint8_t ICMP_PROTO         = 0x01;
static const uint8_t IGMP_PROTO         = 0x02;
static const uint8_t EXPERIMENT1_PROTO  = 0xFD;
static const uint8_t EXPERIMENT2_PROTO  = 0xFE;

// maps socket types to protocol numbers
static const uint8_t IPV4_PROTO[NUM_SOCK_TYPES] =
{
    UDP_PROTO    // IPV4_UDP_SOCK
};

// IP Header struct
typedef struct {
    uint8_t version_ihl;
    uint8_t dscp_ecn;
    uint16_t total_len;
    uint16_t identification;
    uint16_t flags_frag;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src;
    uint32_t dst;
    // options
} IPv4Header_t;

// address is 32 bits
// always assume in system endianness
typedef uint32_t IPv4Addr_t;

/// @brief fill in the 'addr' field with an IPv4 address a.b.c.d
static inline void IPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, IPv4Addr_t* addr) {
    *addr = d;
    *addr |= (c << 8);
    *addr |= (b << 16);
    *addr |= (a << 24);
}

/// @brief calculates IPv4 checksum
/// header checksum field must be zero before calling!
uint16_t checksum(const uint16_t* data, uint16_t len) {
    uint16_t sum = 0;
    for(int i = 0; i < (len / sizeof(uint16_t)); i++) {
        sum += data[i];
    }

    // retrun one's complement
    return ~sum;
}

} // namespace ipv4

#endif
