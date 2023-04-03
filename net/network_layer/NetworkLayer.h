#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#include "net/packet/Packet.h"
#include "net/socket/Socket.h"
#include "return.h"

// all the possible addressing information for all layers
// layers that will never be used together can have their information unioned
typedef struct {
    // Ethernet info
    uint8_t mac[6];

    // IPv4 info
    uint32_t ipv4_addr;

    // UDP info
    uint16_t udp_port;
} netaddr_t;

// describes a packet sent/received over a socket
typedef struct {
    netaddr_t src;
    netaddr_t dst;
    // hint as to whether bad checksums should be ignored
    // primarily used for receiving packets on loopback as it modifies packets
    // without updating the checksum
    bool ignore_checksum;
} netinfo_t;

/// @brief interface for network layer
class NetworkLayer {
public:
    /// @brief provide the layer with a received packet
    ///        pushes the packet up the stack
    /// @param packet   the received packet
    /// @param info     information about the packet to be filled in
    /// @param caller   the layer that called this function one layer before
    /// NOTE: this function should be reentrant and use scheduler macros,
    ///       layers are allowed to block
    ///       this also means this function must be called in a task
    /// @return
    virtual RetType receive(Packet& packet, netinfo_t& info, NetworkLayer* caller) = 0;

    /// @brief send a packet through the stack
    ///        pushes the packet down the stack
    ///        The header position of 'packet' should be at the last allocated header
    ///        The write position should be at the end of the payload
    /// @param packet   the packet to transmit
    /// @param info     information about the packet
    /// @param caller   the layer that called this function one layer before
    /// NOTE: this function should be reentrant and use scheduler macros,
    ///       layers are allowed to block
    ///       this also means this function must be called in a task
    /// @return
    virtual RetType transmit(Packet& packet, netinfo_t& info, NetworkLayer* caller) = 0;

    /// @brief second pass for transmitting a packet
    ///        The header position of 'packet' should be at the last allocated header
    ///        The write position should be at the end of the payload
    virtual RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer* caller) = 0;
};

#endif
