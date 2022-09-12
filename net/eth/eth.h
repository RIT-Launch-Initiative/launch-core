#ifndef ETH_H
#define ETH_H

#include <stdint.h>

#include "net/socket/Socket.h"

namespace eth {

// layer 2 ethernet 2 frame
typedef struct {
    uint8_t dst[6];
    uint8_t src[6];
    // don't include VLAN tag (802.1Q tag)
    uint16_t ethertype; // or length
    // payload
    // fcs is 4 bytes
} EthHeader_t;

// ethertype values
static const uint16_t IPV4_PROTO = 0x0800;
static const uint16_t IPV6_PROTO = 0x86DD;
static const uint16_t ARP_PROTO  = 0x0806;

// maps socket types to protocol numbers
static const uint16_t ETH_PROTO[NUM_SOCK_TYPES] =
{
    IPV4_PROTO    // IPV4_UDP_SOCK
};

};

#endif
