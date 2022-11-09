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

typedef union {
    uint16_t udp;
} sockport_t;


// socket types
typedef enum {
    IPV4_UDP_SOCK = 0,      // IPv4 and UDP
    RAW_IPV4_UDP_SOCK,      // user needs to provide UDP header, sent over IPv4
    PACKET_SOCK,            // only adds link layer header, dst address must be provided, src will be ignored if 0
    NUM_SOCK_TYPES
} socktype_t;

    // describes a packet sent/received over a socket
typedef struct {
    sockaddr_t src;
    sockaddr_t dst;
    socktype_t type;
} sockinfo_t;


/// @brief Socket interface
/// @tparam ADDR    address type the socket uses
template <typename ADDR>
class Socket {
public:
    /// @brief constructor
    Socket() {};

    /// @brief initialize
    /// @return
    virtual RetType init() = 0;

    /// @brief bind a socket to send/receive from an address
    /// this may mean different things depending on the socket type
    virtual RetType bind(ADDR& addr) = 0;

    /// @brief get how much data can be read without blocking
    virtual size_t available() = 0;

    /// @brief get the Maximum Transmit Unit for this socket
    virtual size_t mtu() = 0;

    /// @brief send a packet over this socket
    /// @return
    virtual RetType send(uint8_t* buff, size_t len, ADDR& dst) = 0;

    /// @brief receive a packet over this socket
    /// source address of the packet will be filled into 'src'
    virtual RetType recv(uint8_t* buff, size_t len, ADDR& src) = 0;
};

#endif
