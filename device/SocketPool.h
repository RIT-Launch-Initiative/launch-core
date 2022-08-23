#ifndef SOCKET_POOL_H
#define SOCKET_POOL_H

#include "device/SocketDevice.h"
#include "queue/allocated_queue.h"

// preallocates socket devices
class SocketPool : public Device {
public:
    RetType obtain() {
        // unlimited amount of people can obtain
        return RET_SUCCESS;
    }

    RetType release() {
        // always succeed
        return RET_SUCCESS;
    }

    RetType poll() {
        // poll every device that's currently allocated
        RetType ret = RET_SUCCESS;

        QueueIterator<SocketDevice*> it = m_allocated.iterator();
        SocketDevice** ptr;
        SocketDevice* dev;

        while(1) {
            ptr = *it;

            if(!ptr) {
                break;
            }

            dev = *ptr;

            RetType temp = dev->poll();
            if(RET_SUCCESS != temp) {
                ret = temp;
            }

            ++it;
        }
    }

    /// @brief allocate a socket device
    ///        socket device will be initialized
    /// @return a pointer to the allocated socket, or NULL on failure
    SocketDevice* alloc() {
        SocketDevice** ptr = m_free.peek();

        if(ptr == NULL) {
            return NULL;
        }

        SocketDevice* dev = *ptr;

        // initialize the socket device
        if(RET_SUCCESS != dev->init()) {
            return NULL;
        }

        if(!m_allocated.push(dev)) {
            return NULL;
        }

        m_free.pop();

        return dev;
    }

    /// @brief free a socket device back to the pool
    ///        double frees are not allowed and dangerous!
    /// @return
    // RetType free(SocketDevice* sock) {
    //     RetType ret = sock->close();
    //
    //     if(RET_SUCCESS != ret) {
    //         return ret;
    //     }
    //
    //     if(!m_free.push(sock)) {
    //         return RET_ERROR;
    //     }
    //
    //     return RET_SUCCESS;
    // }

protected:
    /// @brief protected constructor, use alloc::SocketPool to declare
    /// @param name         the name of the pool device
    /// @param free         preallocated queue
    /// @param allocated    preallocated queue
    SocketPool(const char* name, Queue<SocketDevice*>& free,
               Queue<SocketDevice*>& allocated) : Device(name),
                                                  m_free(free),
                                                  m_allocated(allocated) {};

private:
    Queue<SocketDevice*>& m_free;
    Queue<SocketDevice*>& m_allocated;
};

namespace alloc {

/// @brief templated socket pool for implemented socket device types
/// @tparam T       the socket type, must be a derived type of SocketDevice
/// @tparam SIZE    the number of sockets to allocate in the pool
template <typename T, const size_t SIZE>
class SocketPool : public ::SocketPool {
public:
    /// @brief constructor
    /// @param name     name of pool device
    SocketPool(const char* name) :
                    ::SocketPool(name, m_internalFree, m_internalAllocated) {};

    RetType init() {
        // add all sockets to the free queue
        for(size_t i = 0; i < SIZE; i++) {
            if(!m_internalFree.push(&m_sockets[i])) {
                // this should never happen
                // the queue is the same size as the number of sockets
                return RET_ERROR;
            }
        }

        return RET_SUCCESS;
    }
private:
    T m_sockets[SIZE];
    alloc::Queue<SocketDevice*, SIZE> m_internalFree;
    alloc::Queue<SocketDevice*, SIZE> m_internalAllocated;
};

}

#endif
