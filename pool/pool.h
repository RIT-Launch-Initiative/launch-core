/*******************************************************************************
*
*  Name: pool.h
*
*  Purpose: Contains implementation for a fixed size memory pool.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef POOL_H
#define POOL_H

#include <stdlib.h>
#include <stdint.h>

#include "queue/queue.h"
#include "macros.h"

/// @brief descriptor for an object
/// @tparam T   type of object described
template <typename T>
struct PoolDescriptor {
    void* owner;
    bool allocated;
    T obj;
};

/// @brief memory pool of preallocated objects
/// @tparam T       the type of objects in the pool
template <typename T>
class Pool {
public:
    /// @brief allocate an object from the pool
    /// @return the pointer to the object, or NULL on failure
    T* alloc() {
        Node<PoolDescriptor<T>*>* node = m_freeDesc.pop_node();

        if(node == NULL) {
            // no room
            return NULL;
        }

        // this node is unused now
        m_freeNodes.push_node(node);

        node->data->allocated = true;
        return &(node->data->obj);
    }

    /// @brief free an object back to the pool
    /// NOTE: some double frees are caught, but it's possible a double free
    ///       could result in an object being on the free list twice
    /// @return 'true' if the free was successful, 'false' on error
    virtual bool free(T* obj) {
        // check this object is both allocated and from this pool
        PoolDescriptor<T>* desc = container_of(obj, PoolDescriptor<T>, obj);

        if(!desc->allocated) {
            return false;
        }

        if(desc->owner != this) {
            return false;
        }

        Node<PoolDescriptor<T>*>* node = m_freeNodes.pop_node();

        if(node == NULL) {
            return false;
        }

        // offer this descriptor as free now
        desc->allocated = false;
        node->data = desc;
        m_freeDesc.push_node(node);

        return true;
    }
protected:
    /// @brief protected constructor
    ///        use the alloc::Pool constructor directly
    /// @param desc     array of 'size' many preallocated descriptors
    /// @param nodes    array of 'size' many preallocated nodes
    /// @param size     the number of objects in the pool
    Pool(PoolDescriptor<T>* desc, Node<PoolDescriptor<T>*>* nodes, const size_t size) :
                                                        m_desc(desc),
                                                        m_nodes(nodes),
                                                        m_freeDesc(),
                                                        m_freeNodes() {
        // construct the free object list
        for(size_t i = 0; i < size; i++) {
            m_desc[i].owner = this;
            m_desc[i].allocated = false;
            m_nodes[i].data = &(m_desc[i]);
            m_freeDesc.push_node(&m_nodes[i]);
        }
    }

private:
    PoolDescriptor<T>* m_desc;
    Node<PoolDescriptor<T>*>* m_nodes;
    SimpleQueue<PoolDescriptor<T>*> m_freeDesc;  // holds free descriptors, unallocated
    SimpleQueue<PoolDescriptor<T>*> m_freeNodes; // holds free nodes that have their descriptors allocated
};

namespace alloc {

/// @brief memory pool of preallocated objects
/// @tparam T       the type of objects in the pool
/// @tparam SIZE    the number of objects in the pool
template <typename T, const size_t SIZE>
class Pool : public ::Pool<T> {
public:
    /// @brief constructor
    Pool() : ::Pool<T>(m_internalDesc, m_internalNodes, SIZE) {};

private:
    PoolDescriptor<T> m_internalDesc[SIZE];
    Node<PoolDescriptor<T>*> m_internalNodes[SIZE];
};

}

#endif
