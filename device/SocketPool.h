/*******************************************************************************
*
*  Name: SocketPool.h
*
*  Purpose: Manages a pool of sockets. Sockets can be allocated and returned
*           to/from a socket pool.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef SOCKET_POOL_H
#define SOCKET_POOL_H

#include "net/socket/Socket.h"
#include "queue/allocated_queue.h"

// preallocates socket devices
class SocketPool : public Device {
public:
    /// @brief constructor
    SocketPool(const char* name) : Device(name) {};

    /// @brief allocate a socket
    /// @return a pointer to the allocated socket, or NULL on failure
    virtual Socket* alloc() = 0;

    /// @brief return a socket to the pool
    /// @return
    virtual RetType free(Socket* sock) = 0;
};


namespace alloc {

/// @brief preallocated socket pool
/// @tparam SOCK the type of socket in the pool
/// @tparam SIZE number of sockets in the pool
template <typename SOCK, const size_t SIZE>
class SocketPool : public ::SocketPool {
public:
    /// @brief constructor
    SocketPool(const char* name) : m_socks(), ::SocketPool(name) {};

    virtual RetType init() {
        return RET_SUCCESS;
    }

    RetType obtain() {
        // unlimited amount of people can obtain
        return RET_SUCCESS;
    }

    RetType release() {
        // always succeed
        return RET_SUCCESS;
    }

    /// @brief allocate a socket
    ///        socket will be initialized
    /// @return a pointer to the allocated socket, or NULL on failure
    Socket* alloc() {
        SOCK* sock = m_socks.push();

        if(sock) {
            if(RET_SUCCESS == sock->init()) {
                return sock;
            }

            m_socks.remove(sock);
        }

        return NULL;
    }

    /// @brief free an allocated socket
    /// @return
    RetType free(Socket* sock) {
        m_socks.remove(reinterpret_cast<SOCK*>(sock));

        return RET_SUCCESS;
    }

    /// @brief doesn't poll anything
    /// @return
    virtual RetType poll() {
        return RET_SUCCESS;
    }

protected:
    alloc::Queue<SOCK, SIZE> m_socks;
};

} // namespace alloc

#endif
