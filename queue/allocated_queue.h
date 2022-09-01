#ifndef ALLOCATED_QUEUE_H
#define ALLOCATED_QUEUE_H

#include <stdlib.h>
#include <stdint.h>

#include "queue/queue.h"
#include "pool/pool.h"
#include "macros.h"

namespace alloc {

/// @brief preallocated queue
///        methods work directly on stored data type
template <typename T, const size_t SIZE>
class Queue : public ::SimpleQueue<T>, public ::Queue<T> {
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

        ::SimpleQueue<T>::push_node(node);
        return true;
    }

    /// @brief create an object at the end of the queue and get a pointer to it
    /// @return the object, or NULL on error
    T* push() {
        Node<T>* node = m_pool.alloc();

        if(node == NULL) {
            return NULL;
        }

        ::SimpleQueue<T>::push_node(node);
        return &(node->data);
    }

    /// @brief pop an object off the queue, if there is one to pop
    /// @return
    void pop() {
        Node<T>* node =::SimpleQueue<T>::pop_node();

        if(node == NULL) {
            // nothing popped
            return;
        }

        // give the node back to the pool
        // TODO do something with return?
        m_pool.free(node);
    }

    /// @brief peek at the object on the back of the queue
    /// @return the object, or NULL on error
    T* peek() {
        Node<T>* node = ::SimpleQueue<T>::peek_node();

        if(node == NULL) {
            return NULL;
        }

        return &(node->data);
    }

    /// @brief remove an entry from the queue
    /// @param obj  a pointer to the object to remove
    /// NOTE: if 'obj' was not allocated by the queue, bad things will happen
    void remove(T* obj) {
        Node<T>* node = container_of(obj, Node<T>, data);
        ::SimpleQueue<T>::remove_node(node);
    }

    /// @brief get the number of nodes on the queue
    /// @return the size of the queue
    size_t size() {
        return ::SimpleQueue<T>::num_nodes();
    }

    /// @brief get an iterator starting at the head
    /// @return the iterator
    QueueIterator<T> iterator() {
        return QueueIterator<T>{SimpleQueue<T>::head()};
    }

private:
    alloc::Pool<Node<T>, SIZE> m_pool;
};

/// @brief preallocated sorted queue
///        methods work directly on stored data type
template <typename T, const size_t SIZE>
class SortedQueue : public ::SimpleSortedQueue<T>, public ::Queue<T> {
public:
    /// @brief constructor
    explicit SortedQueue(sort_t<T> sort) : m_pool(), ::SimpleSortedQueue<T>(sort) {};

    /// @brief push an object onto the queue
    /// @return 'true' on success, 'false' on error
    bool push(T obj) {
        Node<T>* node = m_pool.alloc();

        if(node == NULL) {
            return false;
        }

        node->data = obj;

        ::SimpleSortedQueue<T>::push_node(node);
        return true;
    }

    // can't preallocate an object because then we can't sort until the pointer is set

    /// @brief pop an object off the queue, if there is one to pop
    /// @return
    void pop() {
        Node<T>* node = ::SimpleSortedQueue<T>::pop_node();

        if(node == NULL) {
            // nothing popped
            return;
        }

        // give the node back to the pool
        // TODO do something with return?
        m_pool.free(node);
    }

    /// @brief peek at the object on the back of the queue
    /// @return the object, or NULL on error
    T* peek() {
        Node<T>* node = ::SimpleSortedQueue<T>::peek_node();

        if(node == NULL) {
            return NULL;
        }

        return &(node->data);
    }

    /// @brief remove an entry from the queue
    /// @param obj  a pointer to the object to remove
    /// NOTE: if 'obj' was not allocated by the queue, bad things will happen
    void remove(T* obj) {
        Node<T>* node = container_of(obj, Node<T>, data);
        ::SimpleSortedQueue<T>::remove_node(node);
    }

    /// @brief get the number of nodes on the queue
    /// @return the size of the queue
    size_t size() {
        return ::SimpleSortedQueue<T>::num_nodes();
    }

    /// @brief get an iterator starting at the head
    /// @return the iterator
    QueueIterator<T> iterator() {
        return QueueIterator<T>{SimpleSortedQueue<T>::head()};
    }

private:
    alloc::Pool<Node<T>, SIZE> m_pool;
};

}

#endif
