#ifndef ROUTER_H
#define ROUTER_H

#include "net/packet/packet.h"
#include "return.h"

// NOTE: idea is you have a stack of routers

/// @brief interface for a packet router
class Router {
public:
    /// @brief provide the router with a received packet
    ///        pushes the packet up the stack
    /// @param router   the router that called this function one layer before,
    ///                 or NULL if this is the top or bottom of the stack
    /// @return
    RetType route(Packet& packet, Router* router) = 0;

    /// @brief send a packet through the router
    ///        pushes the packet down the stack
    /// @param router   the router that called this function one layer before,
    ///                 or NULL if this is the top or bottom of the stack
    /// @return
    RetType transmit(Packet& packet, Router* router) = 0;
};

#endif
