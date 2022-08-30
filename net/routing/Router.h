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
    /// @return
    RetType receive(Packet& packet) = 0;

    /// @brief send a packet through the router
    ///        pushes the packet down the stack
    /// @return
    RetType transmit(Packet& packet) = 0;
};

// /// @brief address-based router
// /// @tparam ADDR    the address type
// template <typename ADDR>
// class AddressRouter : public Router {
// public:
//     enum Route_t {
//         TRANSMIT_ROUTE,
//         RECEIVE_ROUTE
//     };
//
//     /// @brief add a route for packets
//     ///        adds a transmit or receive route
//     ///        when this address is received or transmitter, the packet is
//     ///        pushed to router to
//     /// @param type     the type of route, either transmit or receive
//     /// @param router   the router to push packets to
//     /// @param addr     the address of to subscribe the router to
//     /// @return
//     RetType add_route(Route_t type, Router& router, ADDR addr) = 0;
//
//     /// @brief remove a route
//     /// @param type     the type of route
//     /// @param addr     the address
//     /// @addr   the address to remove
//     RetType remove_route(Route_t type, ADDR addr) = 0;
// };

#endif
