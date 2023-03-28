#include <stdio.h>

#include "net/ipv4/IPv4Router.h"
#include "net/loopback/Loopback.h"
#include "net/simple_arp/SimpleArpLayer.h"
#include "net/network_layer/NetworkLayer.h"
#include "net/icmp/icmp.h"
class Blackhole : public NetworkLayer {
public:
    /// @brief constructor
    Blackhole() {};

    /// @brief transmit
    RetType transmit(Packet &packet, sockinfo_t &, NetworkLayer *) {
        // don't do anything

        return RET_SUCCESS;
    }

    /// @brief transmit (second pass)
    RetType transmit2(Packet &packet, sockinfo_t &, NetworkLayer *) {
        printf("transmitting payload: \n");

        uint8_t *buff = packet.read_ptr<uint8_t>();
        for (size_t i = 0; i < packet.size(); i++) {
            printf("%02x ", buff[i]);
        }
        printf("\n\n");

        return RET_SUCCESS;
    }

    /// @brief receive
    RetType receive(Packet &packet, sockinfo_t &, NetworkLayer *) {
        printf("received payload: \n");

        uint8_t *buff = packet.read_ptr<uint8_t>();
        for (size_t i = 0; i < packet.size(); i++) {
            printf("%02x ", buff[i]);
        }
        printf("\n\n");

        return RET_SUCCESS;
    }
};

int main() {
    ipv4::IPv4Router ip;
    Loopback lo;
    Blackhole b;

    ipv4::IPv4Addr_t addr1;
    ipv4::IPv4Address(10, 10, 10, 5, &addr1);
    
    ipv4::IPv4Addr_t subnet1;
    ipv4::IPv4Address(255, 255, 255, 0, &subnet1);
    
    if(RET_SUCCESS != ip.add_route(addr1, subnet1, lo)) {
        printf("loopback failed");
        return -1;
    }
    auto icmp = icmp::ICMP(ip);
    
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

    if(RET_SUCCESS != icmp.transmit(packet, msg, &b)) {
        printf("Transmit 1 failed\n");
        return -1;
    }
};

