#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include <stdint.h>
#include "net/socket/Socket.h"

namespace udp {
    typedef struct {
        uint16_t length;
        uint16_t checksum;
        uint16_t dst;
        uint16_t src;
    } UDP_HEADER_T;

    uint16_t calc_checksum(UDP_HEADER_T header, uint8_t *payload) {
        int sum = 0;




        return ~sum;
    }

    void compute_udp_checksum(struct iphdr *pIph, unsigned short *ipPayload) {
        register unsigned long sum = 0;
        struct udphdr *udphdrp = (struct udphdr*)(ipPayload);
        unsigned short udpLen = htons(udphdrp->len);
        //printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~udp len=%dn", udpLen);
        //add the pseudo header
        //printf("add pseudo headern");
        //the source ip
        sum += (pIph->saddr>>16)&0xFFFF;
        sum += (pIph->saddr)&0xFFFF;
        //the dest ip
        sum += (pIph->daddr>>16)&0xFFFF;
        sum += (pIph->daddr)&0xFFFF;
        //protocol and reserved: 17
        sum += htons(IPPROTO_UDP);
        //the length
        sum += udphdrp->len;

        //add the IP payload
        //printf("add ip payloadn");
        //initialize checksum to 0
        udphdrp->check = 0;
        while (udpLen > 1) {
            sum += * ipPayload++;
            udpLen -= 2;
        }
        //if any bytes left, pad the bytes and add
        if(udpLen > 0) {
            //printf("+++++++++++++++padding: %dn", udpLen);
            sum += ((*ipPayload)&htons(0xFF00));
        }
        //Fold sum to 16 bits: add carrier to result
        //printf("add carriern");
        while (sum>>16) {
            sum = (sum & 0xffff) + (sum >> 16);
        }
        //printf("one's complementn");
        sum = ~sum;
        //set computation result
        udphdrp->check = ((unsigned short)sum == 0x0000)?0xFFFF:(unsigned short)sum;

    bool verify_checksum(UDP_HEADER_T header, uint8_t *payload, uint16_t checksum) {
        // TODO: Once we figure out pseudo header issue with net stack
        return 0;
    }
}

#endif //LAUNCH_CORE_UDP_H
