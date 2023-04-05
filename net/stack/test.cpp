#include <stdlib.h>
#include <stdio.h>

#include "net/stack/IPv4UDP/IPv4UDPStack.h"
#include "net/stack/IPv4UDP/IPv4UDPSocket.h"
#include "net/network_layer/NetworkLayer.h"


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
    uint8_t msg[5] = {'h', 'e', 'l', 'l', 'o'};
    size_t len = 5;
    uint8_t buff[5];

    Blackhole blackhole;

    IPv4UDPStack stack{10, 10, 10, 1,\
                       255,255,255,0,
                       blackhole};

    if(RET_SUCCESS != stack.init()) {
        printf("failed to initialize network stack\n");
        exit(1);
    }


    IPv4UDPSocket* sock = stack.get_socket();
    if(NULL == sock) {
        printf("failed to get network socket from stack\n");
        exit(1);
    }

    IPv4UDPSocket::addr_t addr;
    addr.ip[0] = addr.ip[1] = addr.ip[2] = addr.ip[3] = 0;
    addr.port = 8000;

    if(RET_SUCCESS != sock->bind(addr)) {
        printf("failed to bind socket\n");
        exit(1);
    }

    addr.ip[0] = 127;
    addr.ip[1] = 0;
    addr.ip[2] = 0;
    addr.ip[3] = 1;
    addr.port = 8000;
    if(RET_SUCCESS != sock->send(msg, len, &addr)) {
        printf("failed to send message on socket\n");
        exit(1);
    }

    if(RET_SUCCESS != sock->recv(buff, &len, &addr)) {
        printf("failed to receive message on socket\n");
        exit(1);
    }

    printf("received: '%c %c %c %c %c' on port %u\n", buff[0], buff[1], buff[2], \
                                                      buff[3], buff[4], addr.port);

    addr.ip[0] = 10;
    addr.ip[1] = 10;
    addr.ip[2] = 10;
    addr.ip[3] = 1;
    addr.port = 8000;
    len = 5;
    if(RET_SUCCESS != sock->send(msg, len, &addr)) {
        printf("failed to send message on socket\n");
        exit(1);
    }

    stack.free_socket(sock);
}
