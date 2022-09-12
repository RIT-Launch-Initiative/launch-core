/*******************************************************************************
*
*  Name: queue_simple.h
*
*  Purpose: Implements a simple queue that operates on nodes with no memory
*           allocation for new nodes.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef QUEUE_SIMPLE_H
#define QUEUE_SIMPLE_H

#include "queue/queue_node.h"
#include "queue/queue_sort.h"

/// @brief FIFO queue implementation that works directly on nodes
/// @tparam T   the object type stored in the queue
template <typename T>
class SimpleQueue {
public:
    /// @brief constructor
    SimpleQueue() : m_head(NULL), m_tail(NULL), m_size(0) {};

    /// @brief push a node onto the front of the queue
    virtual void push_node(Node<T>* node) {
        if(NULL == m_head) {
            // queue is currently empty, this is the new head
            m_head = node;
            m_tail = node;
            node->prev = NULL;
            node->next = NULL;
            m_size = 1;
        } else {
            m_size++;
            m_head->prev = node;
            node->next = m_head;
            node->prev = NULL;
            m_head = node;
        }
    }

    /// @brief pop a node off the back of the queue
    /// @return the node on the end of the queue
    Node<T>* pop_node() {
        Node<T>* ret = m_tail;

        if(ret == NULL) {
            return ret;
        }

        m_tail = ret->prev;
        m_size--;

        if(m_tail == NULL) {
            // we popped off the last element
            m_head = NULL;
        } else {
            m_tail->next = NULL;
        }

        return ret;
    }

    /// @brief remove a node from the queue
    void remove_node(Node<T>* node) {
        if(node == m_head) {
            m_head = node->next;
            m_head->prev = NULL;
        } else if(node == m_tail) {
            m_tail = node->prev;
            m_tail->next = NULL;
        } else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        m_size--;
    }

    /// @brief get the node from the back of the queue without popping it
    /// @return the node on the end of the queue
    Node<T>* peek_node() {
        return m_tail;
    }

    /// @brief get the number of nodes on the queue
    /// @return the size of the queue
    inline size_t num_nodes() {
        return m_size;
    }

    /// @brief get the head node of the queue
    /// @return the head node
    inline Node<T>* head() {
        return m_head;
    }

protected:
    Node<T>* m_head;
    Node<T>* m_tail;
    size_t m_size;
};

/// @brief sorted queue implementation that works directly on nodes
/// @tparam T   the object type stored by the queue
template <typename T>
class SimpleSortedQueue : public SimpleQueue<T> {

using SimpleQueue<T>::m_head;
using SimpleQueue<T>::m_tail;
using SimpleQueue<T>::m_size;

public:
    /// @brief constructor
    explicit SimpleSortedQueue(sort_t<T> sort) : m_sort(sort) {};

    void push_node(Node<T>* node) {
        if(NULL == m_head) {
            // queue is currently empty, this is the new head
            m_head = node;
            m_tail = node;
            node->prev = NULL;
            node->next = NULL;
            m_size = 1;
        } else {
            m_size++;

            // look through all the nodes
            Node<T>* it = m_head;
            while(it != NULL) {
                if(m_sort(it->data, node->data)) {
                    // 'it' is the first node that should be AFTER 'node'
                    break;
                }

                it = it->next;
            }

            if(it == NULL) {
                // our node belongs at the tail
                node->prev = m_tail;
                node->next = NULL;
                m_tail->next = node;
                m_tail = node;
            } else {
                // our node belongs before 'it'
                node->next = it;
                node->prev = it->prev;
                it->prev = node;

                if(it == m_head) {
                    // we replaced the head node
                    m_head = node;
                }
            }
        }
    }
private:
    sort_t<T> m_sort;
};

#endif
