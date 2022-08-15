#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdint.h>

/// @brief queue node
/// @tparam T   the object type stored in the node
template <typename T>
struct Node {
    Node<T>* prev;
    Node<T>* next;
    T data;
};

/// @brief FIFO queue
/// @tparam T   the object type stored in the queue
template <typename T>
class Queue {
public:
    /// @brief constructor
    Queue() : m_head(NULL), m_tail(NULL), m_size(0) {};

    /// @brief push a node onto the front of the queue
    virtual void push(Node<T>* node) {
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
    Node<T>* pop() {
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

    /// @brief get the node from the back of the queue without popping it
    /// @return the node on the end of the queue
    Node<T>* peek() {
        return m_tail;
    }

    /// @brief get the number of nodes on the queue
    /// @return the size of the queue
    inline size_t size() {
        return m_size;
    }

protected:
    Node<T>* m_head;
    Node<T>* m_tail;
    size_t m_size;
};

/// @brief sorting function
///        returns true if fst should be popped before snd (i.e. snd is later in the queue)
template <typename T>
using sort_t = bool (*)(T& fst, T& snd);

/// @brief sorted queue
/// @tparam T   the object type stored by the queue
template <typename T>
class SortedQueue : public Queue<T> {

using Queue<T>::m_head;
using Queue<T>::m_tail;
using Queue<T>::m_size;

public:
    /// @brief constructor
    explicit SortedQueue(sort_t<T> sort) : m_sort(sort), Queue<T>() {};

    void push(Node<T>* node) {
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
