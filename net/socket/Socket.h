#ifndef SOCKET_H
#define SOCKET_H

#include <stdlib.h>
#include <stdint.h>

#include "return.h"

typedef union {
    uint32_t ipv4;
} sockaddr_t;

typedef union {
    uint16_t udp;
} sockport_t;

typedef enum {
    IPV4_UDP_SOCK = 0,
    NUM_SOCK_TYPES
} socktype_t;

// socket message
// user should fill in all fields except 'type', which the Socket object fills in
typedef struct {
    sockaddr_t addr;
    sockport_t port;
    socktype_t type;
    uint8_t* payload;    // payload
    size_t payload_len;  // payload length
} sockmsg_t;


/// @brief network socket device
/// @tparam TYPE type of the socket
template <const socktype_t SOCK>
class Socket {
public:
    /// @brief constructor
    Socket() {};

    /// @brief initialize
    /// @return
    virtual RetType init() = 0;

    /// @brief send a message from the socket
    /// @param msg  the message to send
    ///             'addr.addr' is the destination address
    ///             'addr.port' is the destination port
    ///             'payload' is a buffer holding the payload
    ///             'payload_len' is the length of the payload
    /// @return if all bytes were sent
    virtual RetType sendmsg(sockmsg_t* msg) = 0;

    /// @brief received a message on the socket
    /// @param msg  the message to receive
    ///             'addr.addr' is the source address of the message, filled by function
    ///             'addr.port' is the source port of the message, filled by function
    ///             'payload' is the buffer to copy the payload to
    ///             'payload_len' is the size of the buffer, up to this many bytes are copied
    ///                           this is set to the actual number of bytes copied
    /// @return
    virtual RetType recvmsg(sockmsg_t* msg) = 0;

    /// @brief bind this socket to an address
    ///        the socket will send and receive from this address
    /// NOTE: some platforms may treat the IP address differently,
    ///       for devices where the IP cannot be reconfigured it is ignored
    /// @param addr     the address to bind to
    /// @return
    virtual RetType bind(sockaddr_t* addr) = 0;

    /// @brief subscribe to a multicast group address
    ///        generally only the network address will be used
    /// @param addr     the multicast group address to subscribe to
    /// @return
    virtual RetType subscribe(sockaddr_t* addr) = 0;

    /// @brief close the socket
    /// @return
    virtual RetType close() = 0;

    /// @brief get how much data can currently be read
    /// NOTE: units are up to the device
    ///       for datagrams it may be packets rather than bytes
    /// @return the amount of data that can be read in bytes
    virtual size_t available() = 0;
};

#endif