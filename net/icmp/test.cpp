#include <stdio.h>

#include "net/ipv4/IPv4Router.h"
#include "net/loopback/Loopback.h"
#include "net/simple_arp/SimpleArpLayer.h"
#include "net/network_layer/NetworkLayer.h"

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
    
    ICMP(NetworkLayer& out) : m_out(out) {};

    RetType receive(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();
        icmp_t head;
        packet.read<icmp_t>(&head);

        if(checksum((uint16_t *)&head, sizeof(icmp_t)) != head.checksum) {
            RESET();
            return RET_ERROR;
        }

        uint32_t& dst = info.src.ipv4_addr;
        uint32_t& src = info.dst.ipv4_addr;
        info.src.ipv4_addr = dst;
        info.dst.ipv4_addr = src;

        RetType ret = CALL(m_out.transmit(packet, info, this));
        RESET();
        return ret;
    }

    RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();
        icmp_t* head = packet.allocate_header<icmp_t>();
        head->type = ECHO_MESSAGE;

        RetType ret = CALL(m_out.transmit(packet, info, caller));
        
        RESET();
        return ret;
    }   

    RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();
        icmp_t* head = packet.allocate_header<icmp_t>();
        head->type = ECHO_REPLY;

        if(checksum((uint16_t *)&head, sizeof(icmp_t)) != head->checksum) {
            RESET();
            return RET_ERROR;
        }

        RetType ret = CALL(m_out.transmit(packet, info, this));
        RESET();
        return ret;
    }

private:
    NetworkLayer& m_out;
    uint16_t checksum(uint16_t *b, int len) {
        uint16_t *buf = b;
        uint16_t sum = 0;

        while(len > 1) {
            sum += *buf++;
            len -= 2;
        }

        if(len == 1) {
            sum += *(unsigned char*)buf;
        }

        sum = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);
        return ~sum;
    }


};
}
int main() {
    ipv4::IPv4Router ip;
    Loopback lo;
    NetworkLayer& m_out;

    ipv4::IPv4Addr_t addr1;
    ipv4::IPv4Address(10, 10, 10, 5, &addr1);
    
    ipv4::IPv4Addr_t subnet1;
    ipv4::IPv4Address(255, 255, 255, 0, &subnet1);
    
    if(RET_SUCCESS != ip.add_route(addr1, subnet1, lo)) {
        printf("loopback failed");
        return -1;
    }
    auto icmp = icmp::ICMP(m_out);
    
    if(RET_SUCCESS != ip.add_protocol(ipv4::UDP_PROTO, icmp)) {
        printf("failed to add protocol\n");
        return -1;
    }
    
    alloc::Packet<50, 100> packet;

    sockaddr_t dst;
    dst.ipv4_addr = addr1;
    dst.udp_port = 8000;

    sockinfo_t msg;
    msg.dst = dst;
    msg.type = IPV4_UDP_SOCK;

    if(RET_SUCCESS != icmp.transmit(packet, msg, m_out)) {
        printf("Transmit 1 failed\n");
        return -1;
    }
};

