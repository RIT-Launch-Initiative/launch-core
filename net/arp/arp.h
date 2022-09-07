#ifndef ARP_H
#define ARP_H

#include <stdint.h>

namespace arp {

typedef struct {
    uint16_t htype; // hardware type
    uint16_t ptype; // protocol type
    uint8_t hlen;   // hardware address length
    uint8_t plen;   // protocol address length
    uint16_t oper;  // operation
    uint8_t sha[6]; // sender hardware address
    uint8_t spa[4]; // sender protocol address
    uint8_t tha[6]; // target hardware address
    uint8_t tpa[4]; // target protocol address
} ArpHeader_t;

// hardware type for Ethernet
static const uint16_t ETH_HTYPE = 1;

// protocol type for IPv4
static const uint16_t IPV4_PTYPE = 0x0800;

// request and reply operation
static const uint16_t REQUEST_OPER = 1;
static const uint16_t REPLY_OPER = 2;

}

#endif
