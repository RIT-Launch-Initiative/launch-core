#ifndef SOCKET_DEVICE_H
#define SOCKET_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"
#include "return.h"

/// @brief network socket device
class SocketDevice : public Device {
public:
    typedef struct {
        uint32_t addr;      // network address in system endianness
        uint16_t port;      // transport port in system endianness
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
    /// @return if all bytes were sent
    virtual RetType sendmsg(msg_t* msg) = 0;

    /// @brief received a message on the socket
    /// @param msg  the message to receive
    ///             'addr.addr' is the source address of the message, filled by function
    ///             'addr.port' is the source port of the message, filled by function
    ///             'payload' is the buffer to copy the payload to
    ///             'payload_len' is the size of the buffer, up to this many bytes are copied
    ///                           this is set to the actual number of bytes copied
    /// @return
    virtual RetType recvmsg(msg_t* msg) = 0;

    /// @brief bind this socket to an address
    ///        the socket will send and receive from this address
    /// @param addr     the address to bind to
    /// @return
    virtual RetType bind(addr_t* addr) = 0;

    /// @brief subscribe to a multicast group address
    ///        generally only the network address will be used
    /// @param addr     the multicast group address to subscribe to
    /// @return
    virtual RetType subscribe(addr_t* addr) = 0;

    /// @brief get how much data can currently be read
    /// NOTE: units are up to the device
    ///       for datagrams it may be packets rather than bytes
    /// @return the amount of data that can be read in bytes
    virtual size_t available() = 0;
};

#endif
