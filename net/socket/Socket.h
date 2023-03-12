#ifndef SOCKET_H
#define SOCKET_H

#include <stdlib.h>
#include <stdint.h>

#include "return.h"

// all the possible addressing information for any kind of socket
typedef struct {
    // Ethernet info
    uint8_t mac[6];

    // IPv4 info
    uint32_t ipv4_addr;

    // UDP info
    uint16_t udp_port;
} sockaddr_t;

// describes a packet sent/received over a socket
typedef struct {
    sockaddr_t src;
    sockaddr_t dst;
} sockinfo_t;


/// @brief Socket interface
/// @tparam ADDR    address type the socket uses
template <typename ADDR>
class Socket {
public:
    /// @brief bind a socket to send/receive from an address
    /// this may mean different things depending on the socket type
    /// @return
    virtual RetType bind(ADDR& addr) = 0;

    /// @brief unbind a socket
    /// @return
    virtual RetType unbind() = 0;

    /// @brief get the Maximum Transmit Unit for this socket
    /// @return the MTU, in bytes
    virtual size_t mtu() = 0;

    /// @brief get how much data can be read without blocking
    /// @return units vary by socket type
    virtual size_t available() = 0;

    /// @brief send a packet over this socket
    /// @return
    virtual RetType send(uint8_t* buff, size_t len, ADDR& dst) = 0;

    /// @brief receive a packet over this socket
    /// source address of the packet will be filled into 'src'
    /// @return
    virtual RetType recv(uint8_t* buff, size_t len, ADDR& src) = 0;
};

#endif
