#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>

namespace ipv4 {

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
} header_t;

// address is 32 bits
typedef uint32_t addr_t;

/// @brief fill in the 'addr' field with an IPv4 address a.b.c.d
static inline void IPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, addr_t* addr) {
    addr = d;
    addr |= (c << 8);
    addr |= (b << 16);
    addr |= (a << 24);
}

} // namespace ipv4

#endif
