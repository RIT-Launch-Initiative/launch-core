/*******************************************************************************
*
*  Name: queue.h
*
*  Purpose: Defines the interface for a queue.
*           The queues in 'allocated_queue.h' implement this interface.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdint.h>

#include "queue/queue_node.h"
#include "queue/queue_iterator.h"
#include "queue/queue_simple.h"

/// @brief interface for a queue
/// @tparam T   the object type stored in the queue
template <typename T>
class Queue {
public:
    /// @brief push an object onto the queue
    /// @return 'true' on success, 'false' on error
    virtual bool push(T obj) = 0;

    /// @brief pop an object off the queue, if there is one to pop
    /// @return
    virtual void pop() = 0;

    /// @brief peek at the object on the back of the queue
    /// @return the object, or NULL on error
    virtual T* peek() = 0;

    /// @brief remove an entry from the queue
    /// @param obj  a pointer to the object to remove
    /// @return
    virtual void remove(T* obj) = 0;

    /// @brief get the number of nodes on the queue
    /// @return the size of the queue
    virtual size_t size() = 0;

    /// @brief get an iterator starting at the head
    /// @return the iterator
    virtual QueueIterator<T> iterator() = 0;
};

#endif
