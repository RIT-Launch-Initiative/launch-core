#ifndef ALLOCATED_QUEUE_H
#define ALLOCATED_QUEUE_H

#include <stdlib.h>
#include <stdint.h>

#include "queue/queue.h"
#include "pool/pool.h"

namespace alloc {

/// @brief preallocated queue
///        methods work directly on stored data type
template <typename T, const size_t SIZE>
class Queue : public ::Queue<T> {
public:
    /// @brief constructor
    Queue() : m_pool() {};

    /// @brief push an object onto the queue
    /// @return 'true' on success, 'false' on error
    bool push(T obj) {
        Node<T>* node = m_pool.alloc();

        if(node == NULL) {
            return false;
        }

        node->data = obj;

        ::Queue<T>::push(node);
        return true;
    }

    /// @brief pop an object off the queue, copying it into 'obj'
    /// @return 'true' on success, 'false' on error
    bool pop(T* obj) {
        Node<T>* node = ::Queue<T>::pop();

        if(node == NULL) {
            // nothing popped
            return false;
        }

        // copy out
        *obj = node->data;

        // give the node back to the pool
        return m_pool.free(node);
    }

    /// @brief peek at the object on the back of the queue
    /// @return the object, or NULL on error
    T* peek() {
        const Node<T>* node = ::Queue<T>::peek();

        if(node == NULL) {
            return NULL;
        }

        return &(node->data);
    }

private:
    Pool<Node<T>, SIZE> m_pool;
};

/// @brief preallocated sorted queue
///        methods work directly on stored data type
template <typename T, const size_t SIZE>
class SortedQueue : public ::SortedQueue<T> {
public:
    /// @brief constructor
    explicit SortedQueue(sort_t<T> sort) : m_pool(), ::SortedQueue<T>(sort) {};

    /// @brief push an object onto the queue
    /// @return 'true' on success, 'false' on error
    bool push(T obj) {
        Node<T>* node = m_pool.alloc();

        if(node == NULL) {
            return false;
        }

        node->data = obj;

        ::SortedQueue<T>::push(node);
        return true;
    }

    /// @brief pop an object off the queue, copying it into 'obj'
    /// @return 'true' on success, 'false' on error
    bool pop(T* obj) {
        Node<T>* node = ::SortedQueue<T>::pop();

        if(node == NULL) {
            // nothing popped
            return false;
        }

        // copy out
        *obj = node->data;

        // give the node back to the pool
        return m_pool.free(node);
    }

    /// @brief peek at the object on the back of the queue
    /// @return the object, or NULL on error
    T* peek() {
        Node<T>* node = ::SortedQueue<T>::peek();

        if(node == NULL) {
            return NULL;
        }

        return &(node->data);
    }

private:
    Pool<Node<T>, SIZE> m_pool;
};

}

#endif
