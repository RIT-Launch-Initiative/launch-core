#ifndef POOL_H
#define POOL_H

#include <stdlib.h>
#include <stdint.h>

#include "queue/queue.h"

/// @brief memory pool of preallocated objects
/// @tparam T       the type of objects in the pool
template<typename T>
class Pool {
public:
    /// @brief allocate an object from the pool
    /// @return the pointer to the object, or NULL on failure
    T* alloc() {
        Node<T*>* node = m_freeObjs.pop_node();

        if(node == NULL) {
            // no room
            return NULL;
        }

        m_freeNodes.push_node(node);

        return node->data;
    }

    /// @brief free an object back to the pool
    /// NOTE: if an object is freed twice, bad things will happen
    ///       the object will be offered in two nodes on the freeObjs list
    /// @return 'true' if the free was successful, 'false' on error
    virtual bool free(T* obj) {
        Node<T*>* node = m_freeNodes.pop_node();

        if(node == NULL) {
            return false;
        }

        // offer this object as free now
        node->data = obj;
        m_freeObjs.push_node(node);

        return true;
    }
protected:
    /// @brief protected constructor
    ///        use the alloc::Pool constructor directly
    /// @param objs     array of 'size' many preallocated objects
    /// @param nodes    array of 'size' many preallocated nodes
    /// @param size     the number of objects in the pool
    Pool(T* objs, Node<T*>* nodes, const size_t size) : m_objs(objs),
                                                        m_nodes(nodes),
                                                        m_freeObjs(),
                                                        m_freeNodes() {
        // construct the free object list
        for(size_t i = 0; i < size; i++) {
            m_nodes[i].data = &(m_objs[i]);
            m_freeObjs.push_node(&m_nodes[i]);
        }
    }

private:
    T* m_objs;
    Node<T*>* m_nodes;
    SimpleQueue<T*> m_freeObjs;  // holds free objects
    SimpleQueue<T*> m_freeNodes; // holds free nodes not associated with objects
};

namespace alloc {

/// @brief memory pool of preallocated objects
/// @tparam T       the type of objects in the pool
/// @tparam SIZE    the number of objects in the pool
template <typename T, const size_t SIZE>
class Pool : public ::Pool<T> {
public:
    /// @brief constructor
    Pool() : ::Pool<T>(m_internalObjs, m_internalNodes, SIZE) {};

private:
    T m_internalObjs[SIZE];
    Node<T*> m_internalNodes[SIZE];
};

}

#endif
