#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>

#include <return.h>

/// @brief fixed size vector that holds objects
/// @tparam T   the object type
template <typename T>
class Vector {
public:
    /// @brief insert an element onto the vector
    /// @param index    the index to insert the element at
    /// @return a pointer to the inserted element, or NULL on error
    T* insert(size_t index) {
        if(index > m_len || m_len == m_capacity) {
            // we can't insert here (or at all if it's full)
            return NULL;
        }

        // right shift by 1
        rshift(index);

        return &(m_elements[index]);
    }

    /// @brief remove an element
    /// @param index    the index of the element to remove
    /// @return
    RetType remove(size_t index) {
        if(index >= m_len) {
            // nothing here to remove
            return RET_ERROR;
        }

        // left shift by 1
        lshift(index);

        return RET_SUCCESS;
    }

    /// @brief push to the back of the vector
    /// @return a pointer to the new object
    T* push_back() {
        return insert(m_len);
    }

    /// @brief remove the last element of the vector
    /// @return
    RetType pop_back() {
        return remove(m_len - 1);
    }

    /// @brief push to the front of the vector
    /// @return a pointer to the new object
    T* push_front() {
        return insert(0);
    }

    /// @brief remove the first element of the vector
    /// @return
    RetType pop_front() {
        return remove(0);
    }

    /// @brief determine if the vector is full
    /// @return 'true' if the vector is full
    bool full() {
        return m_len == m_capacity;
    }

    /// @brief get the current size of the vector
    /// @return the current size, in objects
    size_t size() {
        return m_len;
    }

    /// @brief get the maximum capacity of the vector
    /// @return the maximum capacity, in objects
    size_t capacity() {
        return m_capacity;
    }

    /// @brief get the object at an index
    /// @param index    the index of the object
    /// @return a pointer to the object, or NULL on error
    inline T* operator[](size_t index) {
        if(index < m_len) {
            return &(m_elements[index]);
        }

        return NULL;
    }

protected:
    /// @brief protected constructor, use alloc::Vector instead
    /// @param objs     list of preallocated objects
    /// @param size     number of objects in 'objs'
    Vector(T* objs, size_t size) : m_elements(objs), m_capacity(size), m_len(0) {};

    size_t m_capacity;
    size_t m_len;
    T* m_elements;

    /// @brief helper function to right shift the elements
    /// @param index    index to start shifting at
    void rshift(size_t index) {
        m_len++;

        T last = m_elements[index];
        T temp;

        for(size_t i = index + 1; i < m_len; i++) {
            temp = m_elements[i];
            m_elements[i] = last;
            last = temp;
        }
    }

    /// @brief helper function to left shift the elements
    /// @param index    index to start shifting at
    void lshift(size_t index) {
        m_len--;

        for(size_t i = index; i < m_len; i++) {
            m_elements[i] = m_elements[i + 1];
        }
    }
};


namespace alloc {

/// @brief a preallocated vector
/// @tparam T       the object type held by the vector
/// @tparam SIZE    the number of objects held in the vector
template <typename T, const size_t SIZE>
class Vector : public ::Vector<T> {

public:
    /// @brief constructor
    Vector() : ::Vector<T>(m_internalObjs, SIZE) {};

private:
    T m_internalObjs[SIZE];
};

} // namespace alloc

#endif
