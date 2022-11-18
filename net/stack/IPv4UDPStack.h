/*******************************************************************************
*
*  Name: IPv4UDPStack.h
*
*  Purpose: Implements an IPv4/UDP networking stack
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/

#ifndef IPV4_UDP_STACK_H
#define IPV4_UDP_STACK_H

#include "return.h"
#include "net/socket/socket.h"
#include "net/ipv4/IPv4Router.h"

class IPv4UDPStack;

/// @brief IPv4/UDP address
typedef struct {
    uint32_t addr;  // IPv4 address
    uint16_t port;  // UDP port
} sockaddr_t;

/// @brief socket class
class StackSocket : public Socket<sockaddr_t> {
public:
    /// @brief constructor
    /// @param associated network stack
    StackSocket(IPv4UDPStack& stack) {};

    /// @brief initialize
    /// @return
    RetType init() {
        return RET_SUCCESS;
    }

    /// @brief bind this socket

private:
    //
};

class IPv4UDPStack {
public:
    /// @brief constructor
    IPv4UDPStack() {};

    /// address type for IPv4 / UDP sockets
    typedef struct {
        uint32_t addr;  // IPv4 address (big endian)
        uint16_t port;  // UDP port
    } addr_t;

    /// @brief get a socket from the stack
    /// @return a ponter to the socket, or NULL on error
    Socket<addr_t>* get_socket() {
        // TODO get a socket from a socket pool

        return NULL;
    }

    /// @brief return a socket to the stack
    /// @return
    RetType destroy(Socket<addr_t>* sock) {
        // TODO return a socket to the pool

        return RET_SUCCESS;
    }

    /// @brief bind a socket to receive/send to/from an address
    ///        if addr.ip is 0, all interfaces will be received from
    /// @return
    RetType bind(Socket<addr_t>* sock, addr_t* addr) {
        // TODO bind a socket to receive and send from an address

        return RET_SUCCESS;
    }

    RetType subscribe(Socket<addr_t>* sock, addr_t* addr) {
        // TODO subscribe a socket to receive from a multicast address

        return RET_SUCCESS;
    }


private:
    // IPv4 Router
    ipv4::IPv4Router m_ip;

    // UDP Router
    UDPRouter m_udp;
};

#endif
