#ifndef QUEUE_ITERATOR_H
#define QUEUE_ITERATOR_H

#include "queue/queue_node.h"

/// @brief queue iterator
// TODO inherit from generic Iterator?
/// @tparam T   type that queue stores
template <typename T>
class QueueIterator {
public:
    /// @brief constructor
    /// @param start    pointer to the starting node
    QueueIterator(Node<T>* start) : m_curr(start) {};

    /// @brief iterate
    /// @return the next element in the queue, or NULL if read past the tail
    T* operator++() {
        if(!m_curr) {
            return NULL;
        }

        m_curr = m_curr->next;

        if(m_curr) {
            return &(m_curr->data);
        }

        // we just read the tail
        return NULL;
    }

    /// @brief get the current value
    T* operator*() {
        if(!m_curr) {
            return NULL;
        }

        return &(m_curr->data);
    }

private:
    // current node
    Node<T>* m_curr;
};

#endif
