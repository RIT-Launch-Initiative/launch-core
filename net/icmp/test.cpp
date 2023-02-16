#include <stdio.h>

#include "net/ipv4/IPv4Router.h"
#include "net/loopback/Loopback.h"
#include "net/simple_arp/SimpleArpLayer.h"
#include "net/network_layer/NetworkLayer.h"

class Echo : public NetworkLayer {
public: 
    
    Echo() {};
    Echo(NetworkLayer& out) : m_out(out) {};
    
    RetType recieve(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();
        uint32_t& src = info.src.ipv4_addr;
        uint32_t& dst = info.dst.ipv4_addr;
        printf("src ip address:%u ", src);
        printf("dst ip address:%u ", dst);

        if (dst == src) {
            RetType ret = CALL(m_out.transmit(packet, info, caller));
            RESET();
            return ret;
        }
    }

    RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        RESUME();
        RetType ret = CALL(m_out.transmit(packet, info, caller));
        RESET();
        return ret;
    }   

    RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        return RET_ERROR;
    }

private:
    NetworkLayer& m_out;
};

int main() {
    ipv4::IPv4Router ip;
    Loopback lo;
    Echo e;

    ipv4::IPv4Addr_t addr1;
    ipv4::IPv4Address(10, 10, 10, 5, &addr1);
    
    ipv4::IPv4Addr_t subnet1;
    ipv4::IPv4Address(255, 255, 255, 0, &subnet1);
    
    if(RET_SUCCESS != ip.add_route(addr1, subnet1, lo)) {
        printf("loopback failed");
        return -1;
    }

    if(RET_SUCCESS != ip.add_protocol(ipv4::UDP_PROTO, e)) {
        printf("add protocol failed");
        return -1;
    }


    alloc::Packet<50, 100> packet;

    sockaddr_t dst;
    dst.ipv4_addr = addr1;
    dst.udp_port = 8000;

    sockinfo_t msg;
    msg.dst = dst;
    msg.type = IPV4_UDP_SOCK;

    if(RET_SUCCESS != ip.receive(packet, msg, NULL)) {
        printf("failed recieve");
        return -1;
    }

    if(RET_SUCCESS != ip.transmit(packet, msg, NULL)) {
        printf("Transmit failed");
        return -1;
    }
};

