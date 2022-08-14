#ifndef SOCKETDEVICE_H
#define SOCKETDEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"

/// @brief network socket device
class SocketDevice : public Device {
public:
    typedef struct {
        uint32_t addr;      // network address
        uint16_t port;      // transport port
    } addr_t;

    typedef struct {
        addr_t addr;
        uint8_t* payload;   // payload
        size_t payload_len; // payload length
    } msg_t;

    /// @brief send a message from the socket
    /// @param msg  the message to send
    ///             'addr.addr' is the destination address
    ///             'addr.port' is the destination port
    ///             'payload' is a buffer holding the payload
    ///             'payload_len' is the length of the payload
    /// @return the actual number of bytes sent
    virtual size_t sendmsg(msg_t* msg) = 0;

    /// @brief received a message on the socket
    /// @param msg  the message to receive
    ///             'addr.addr' is the source address of the message, filled by function
    ///             'addr.port' is the source port of the message, filled by function
    ///             'payload' is the buffer to copy the payload to
    ///             'payload_len' is the size of the buffer, up to this many bytes are copied
    /// @return the actual number of bytes received, may be greater than 'payload_len'
    virtual size_t recvmsg(msg_t* msg) = 0;

    /// @brief bind this socket to an address
    ///        the socket will send and receive from this address
    /// @param addr     the address to bind to
    /// @return 'true' on success, 'false' on error
    virtual bool bind(addr_t* addr) = 0;

    /// @brief subscribe to a multicast group address
    ///        generally only the network address will be used
    /// @param addr     the multicast group address to subscribe to
    /// @return 'true' on success, 'false' on error
    virtual bool subscribe(addr_t* addr) = 0;
};

#endif
