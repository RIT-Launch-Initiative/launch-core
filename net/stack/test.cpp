#include <stdio.h>

#include "net/ipv4/IPv4Router.h"
#include "net/udp/UDPRouter.h"
#include "net/loopback/Loopback.h"

// network layer that absorbs all packets and just prints them out
class Blackhole : public NetworkLayer {
public:
    /// @brief constructor
    Blackhole() {};

    /// @brief transmit
    RetType transmit(Packet& packet, sockmsg_t&, NetworkLayer*) {
        printf("transmitting payload: \n");

        uint8_t* buff = packet.read_ptr<uint8_t>();
        for(size_t i = 0; i < packet.size(); i++) {
            printf("%02x ", buff[i]);
        }
        printf("\n\n");

        return RET_SUCCESS;
    }

    /// @brief receive
    RetType receive(Packet& packet, sockmsg_t&, NetworkLayer*) {
        printf("received payload: \n");

        uint8_t* buff = packet.read_ptr<uint8_t>();
        for(size_t i = 0; i < packet.size(); i++) {
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
    udp::UDPRouter udp;

    ipv4::IPv4Addr_t addr1;
    ipv4::IPv4Address(10, 10, 10, 5, &addr1);


    ipv4::IPv4Addr_t subnet1;
    ipv4::IPv4Address(255, 255, 255, 0, &subnet1);

    if(RET_SUCCESS != ip.add_route(addr1, subnet1, lo)) {
        printf("failed to add loopback route\n");
        return -1;
    }


    if (RET_SUCCESS != udp.subscribePort(ip, 8000)) {
        printf("Failed to add subscribe to UDP port\n");
        return -1;
    }

    udp.setTransmitLayer(ip);

    if (RET_SUCCESS != udp.bind(b, 2570)) { // TODO: Figuring out how 8000 becomes 2570 exactly
        printf("Failed to bind layer to UDP port");
        return -1;
    }

    if(RET_SUCCESS != ip.add_protocol(ipv4::UDP_PROTO, udp)) {
        printf("failed to add protocol\n");
        return -1;
    }


    uint8_t buff[50];
    for(size_t i = 0; i < 50; i++) {
        buff[i] = i;
    }

    alloc::Packet<50, 100> packet;

    if(RET_SUCCESS != packet.push(buff, 50)) {
        printf("failed to push to packet\n");
        return -1;
    }

    sockmsg_t msg = {addr1, 8000, IPV4_UDP_SOCK, buff, 50};

    if(RET_SUCCESS != udp.transmit(packet, msg, NULL)) {
        printf("failed to transmit packet\n");
        return -1;
    }

    printf("done\n");
}
