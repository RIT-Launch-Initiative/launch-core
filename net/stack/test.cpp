#include <stdio.h>

#include "net/ipv4/IPv4Router.h"
#include "net/udp/UDPRouter.h"
#include "net/loopback/Loopback.h"
#include "net/simple_arp/SimpleArpLayer.h"

// network layer that absorbs all packets and just prints them out
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

    if (RET_SUCCESS != ip.add_route(addr1, subnet1, lo)) {
        printf("failed to add loopback route\n");
        return -1;
    }

    auto udp = udp::UDPRouter(ip);

    if (RET_SUCCESS != udp.subscribe_port(b, 8000)) {
        printf("Failed to bind layer to UDP port");
        return -1;
    }

    if (RET_SUCCESS != ip.add_protocol(ipv4::UDP_PROTO, udp)) {
        printf("failed to add protocol\n");
        return -1;
    }


    uint8_t buff[50];
    for (size_t i = 0; i < 50; i++) {
        buff[i] = i;
    }

    alloc::Packet<50, 100> packet;

    if (RET_SUCCESS != packet.push(buff, 50)) {
        printf("failed to push to packet\n");
        return -1;
    }

    sockaddr_t dst;
    dst.ipv4_addr = addr1;
    dst.udp_port = 8000;

    sockinfo_t msg;
    msg.dst = dst;
    msg.type = IPV4_UDP_SOCK;

    if (RET_SUCCESS != udp.transmit(packet, msg, &b)) {
        printf("failed to transmit packet (first pass)\n");
        return -1;
    }

    // the stack is responsible for resetting the headers
    packet.seek_header();

    if (RET_SUCCESS != udp.transmit2(packet, msg, &b)) {
        printf("failed to transmit packet (second pass)\n");
        return -1;
    }

    printf("done\n");
}
