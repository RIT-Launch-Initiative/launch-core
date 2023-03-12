#ifndef ETH_H
#define ETH_H

#include <stdint.h>

#include "net/socket/Socket.h"
#include "net/eth/crctable.c"

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
static const uint16_t EXP_PROTO  = 0x0101;

// outputs CRC32 Frame Check Sequence in network order
// Uses Ethernet polynomial
// CRC algorithm and table from Rocksoft
// https://github.com/gburca/RocksoftCRC
// The source is well documented if you're confused what a CRC is
static inline uint32_t calculate_fcs(uint8_t* data, size_t len) {
    const uint32_t CRC_INIT = 0xffffffffL;
    const uint32_t XO_ROT   = 0xffffffffL;

    uint32_t crc = CRC_INIT;

    for(size_t i = 0; i < len; i++) {
        crc = crctable[(crc ^ data[i]) & 0xFFL] ^ (crc >> 8);
    }
    crc = crc ^ XO_ROT;

    return hton32(crc);
}

};

#endif
