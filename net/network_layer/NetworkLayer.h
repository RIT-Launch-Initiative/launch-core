#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#include "net/packet/Packet.h"
#include "net/socket/Socket.h"
#include "return.h"


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
    virtual RetType receive(Packet& packet, sockmsg_t& info, NetworkLayer* caller) = 0;

    /// @brief send a packet through the stack
    ///        pushes the packet down the stack
    /// @param packet   the packet to transmit
    /// @param info     information about the packet
    /// @param caller   the layer that called this function one layer before
    /// NOTE: this function should be reentrant and use scheduler macros,
    ///       layers are allowed to block
    ///       this also means this function must be called in a task
    /// @return
    virtual RetType transmit(Packet& packet, sockmsg_t& info, NetworkLayer* caller) = 0;
};

#endif
