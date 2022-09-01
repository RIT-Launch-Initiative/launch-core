#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#include "net/packet/packet.h"
#include "net/Socket.h"
#include "return.h"

// NOTE: idea is you have a stack of routers

/// @brief interface for network layer
class NetworkLayer {
public:
    /// @brief provide the layer with a received packet
    ///        pushes the packet up the stack
    /// @param packet   the received packet
    /// @param info     information about the packet to be filled in
    /// @param caller   the layer that called this function one layer before
    /// @return
    RetType receive(Packet& packet, msg_t& info, NetworLayer* caller) = 0;

    /// @brief send a packet through the stack
    ///        pushes the packet down the stack
    /// @param packet   the packet to transmit
    /// @param info     information about the packet
    /// @param caller   the layer that called this function one layer before
    /// @return
    RetType transmit(Packet& packet, msg_t& info, NetworkLayer* caller) = 0;
};

#endif
