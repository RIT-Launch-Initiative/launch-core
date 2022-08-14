#ifndef POOL_H
#define POOL_H

#include <stdlib.h>
#include <stdint.h>

#include "queue/queue.h"

/// @brief memory pool of preallocated objects
/// @tparam T       the type of objects in the pool
/// @tparam SIZE    the number of objects allocated in the pool
template<typename T, const size_t SIZE>
class Pool {
public:
    /// @brief constructor
    Pool() : m_freeObjs(), m_freeNodes() {
        // construct the free object list
        for(size_t i = 0; i < SIZE; i++) {
            m_nodes[i].data = &(m_objs[i]);
            m_freeObjs.push(&m_nodes[i]);
        }
    }

    /// @brief allocate an object from the pool
    /// @return the pointer to the object, or NULL on failure
    T* alloc() {
        Node<T*>* node = m_freeObjs.pop();

        if(node == NULL) {
            // no room
            return NULL;
        }

        m_freeNodes.push(node);

        return node->data;
    }

    /// @brief free an object back to the pool
    /// NOTE: if an object is freed twice, bad things will happen
    ///       the object will be offered in two nodes on the freeObjs list
    /// @return 'true' if the free was successful, 'false' on error
    bool free(T* obj) {
        Node<T*>* node = m_freeNodes.pop();

        if(node == NULL) {
            return false;
        }

        // offer this object as free now
        node->data = obj;
        m_freeObjs.push(node);

        return true;
    }
private:
    T m_objs[SIZE];
    Node<T*> m_nodes[SIZE];
    Queue<T*> m_freeObjs;  // holds free objects
    Queue<T*> m_freeNodes; // holds free nodes not associated with objects
};

#endif
