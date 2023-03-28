/*******************************************************************************
* 
*  Name: icmp.h
* 
*  Purpose: Process echo requests and implement ping
* 
*  Author: Chloe Clark
* 
*  RIT Launch Initiative
* 
********************************************************************************/
#ifndef ICMP_H
#define ICMP_H

#include "net/packet/Packet.h"
#include "net/network_layer/NetworkLayer.h"
#include "sched/macros.h"

typedef enum {
    ECHO_MESSAGE = 8,
    ECHO_REPLY = 0
} echo_t;

typedef struct {
    uint8_t type;
    uint8_t code = 0;
    uint16_t checksum = 0;
    uint16_t identifier = 0;
    uint16_t seqNum = 0;
} icmp_t;


namespace icmp {
    class ICMP : public NetworkLayer {
    public:

        /// @brief constructor
        /// @param out  the network layer to send packets to
        ICMP(NetworkLayer &out) : m_out(out) {};


        /// @brief receive  Get an echo request
        /// @return
        RetType receive(Packet &packet, sockinfo_t &info, NetworkLayer *caller) {
            RESUME();
            icmp_t head;
            packet.read<icmp_t>(&head);

            if (checksum((uint16_t *) &head, sizeof(icmp_t)) != head.checksum) {
                return RET_ERROR;
            }

            uint32_t &dst = info.src.ipv4_addr;
            uint32_t &src = info.dst.ipv4_addr;
            info.src.ipv4_addr = dst;           // Switch Src and dst addresses
            info.dst.ipv4_addr = src;


            RetType ret = CALL(m_out.transmit(packet, info, this));
            RESET();
            return ret;

        }

        /// @brief transmit the packet, echo reply
        /// @return
        RetType transmit(Packet &packet, sockinfo_t &info, NetworkLayer *caller) {
            RESUME();
            icmp_t *head_pack = packet.allocate_header<icmp_t>();
            head_pack->type = ECHO_MESSAGE;

            RetType ret = CALL(m_out.transmit(packet, info, this));

            RESET();
            return ret;
        }

        RetType transmit2(Packet &packet, sockinfo_t &info, NetworkLayer *caller) {
            RESUME();
            icmp_t *head = packet.allocate_header<icmp_t>();
            head->type = ECHO_REPLY;

            if (checksum((uint16_t *) &head, sizeof(icmp_t)) != head->checksum) {
                return RET_ERROR;
            }

            RetType ret = CALL(m_out.transmit(packet, info, this));
            RESET();
            return ret;
        }

    private:
        NetworkLayer &m_out;

        uint16_t checksum(uint16_t *b, int len) {
            uint16_t *buf = b;
            uint16_t sum = 0;

            while (len > 1) {
                sum += *buf++;
                len -= 2;
            }

            if (len == 1) {
                sum += *(unsigned char *) buf;
            }

            sum = (sum >> 16) + (sum & 0xFFFF);
            sum += (sum >> 16);
            return ~sum;
        }

    };
}
#endif
