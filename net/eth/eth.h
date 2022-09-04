#ifndef ETH_H
#define ETH_H

#include <stdint.h>

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

};

#endif
