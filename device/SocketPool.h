#ifndef SOCKET_POOL_H
#define SOCKET_POOL_H

#include "device/SocketDevice.h"
#include "queue/allocated_queue.h"

// preallocates socket devices
class SocketPool : public Device {
public:
    /// @brief allocate a socket
    /// @return a pointer to the allocated socket, or NULL on failure
    Socket* alloc() = 0;

    /// @brief return a socket to the pool
    /// @return
    RetType free(Socket* sock) = 0;
};

}

#endif
