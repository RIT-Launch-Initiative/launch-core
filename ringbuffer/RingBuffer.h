/*******************************************************************************
*
*  Name: RingBuffer.h
*
*  Purpose: Implements a fixed size circular queue / ring buffer.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "return.h"

/// @brief ring buffer
class RingBuffer {
public:
    /// @brief push data into the buffer
    /// @param buff     the data to push
    /// @param len      the number of bytes to push
    /// @return the number of bytes pushed
    /// Will push at most 'len' bytes onto the buffer
    /// if overwrite is enabled, will guaranteed push 'len' bytes
    size_t push(uint8_t *buff, size_t len) {
        if (m_len + len > m_size) {
            if (m_overwrite) {
                // move the head back to make room
                m_head = (len + m_tail) % m_size;
            } else {
                // copy less data
                len = m_size - m_len;
                m_len += len;
            }
        } else {
            m_len += len;
        }

        size_t i = 0;
        while (i < len) {
            m_buff[m_tail] = buff[i];
            i++;
            m_tail = (m_tail + 1) % m_size;
        }

        return i;
    }

    /// @brief push data intto the buffer
    /// @tparam T    the type of the object to push
    /// @param obj   the data to be pushed
    /// @return
    template<typename T>
    RetType push(T *obj) {
        if (sizeof(T) != push(reinterpret_cast<uint8_t *>(obj), sizeof(T))) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    /// @brief pop data off of the buffer
    /// @param buff     the buffer to copy in to
    /// @param len      the size of the buffer in bytes
    /// @return the actual number of bytes popped
    /// Will pop at most 'len' bytes, but will pop less if there is less data
    /// in the buffer
    size_t pop(uint8_t *buff, size_t len) {
        size_t i = 0;
        while (i < len) {
            buff[i] = m_buff[m_head];
            i++;
            m_head = (m_head + 1) % m_size;
            if (m_head == m_tail) {
                break;
            }
        }

        m_len -= i;
        return i;
    }

    /// @brief pop data off the buffer
    /// @tparam T   the type to pop it as
    /// @param obj  the object to copy the data into
    /// @return
    template<typename T>
    RetType pop(T *obj) {
        if (sizeof(T) != pop(reinterpret_cast<uint8_t *>(obj), sizeof(T))) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    /// @brief get the current size of the buffer
    /// @return the size of the buffer in bytes
    size_t size() {
        return m_len;
    }

    /// @brief get the maximum capacity of the buffer
    /// @return the maximum capacity in bytes
    size_t capacity() {
        return m_size;
    }

protected:
    /// @brief protected constructor, use alloc::RingBuffer to declare
    RingBuffer(uint8_t *buff, size_t size, bool overwrite) : m_head(0),
                                                             m_tail(0),
                                                             m_len(0),
                                                             m_size(size),
                                                             m_buff(buff),
                                                             m_overwrite(overwrite) {};

private:
    size_t m_head;
    size_t m_tail;
    size_t m_len;

    size_t m_size; // size of the buffer
    uint8_t *m_buff;

    bool m_overwrite;
};

namespace alloc {

/// @brief ring buffer
/// @tparam SIZE            the size in bytes of the buffer
/// @tparam OVERWRITABLE    if the buffer is overwritable,
///                         e.g. writing past head moves the head over old data
    template<const size_t SIZE, const bool OVERWRITABLE>
    class RingBuffer : public ::RingBuffer {
    public:
        /// @brief constructor
        RingBuffer() : ::RingBuffer(m_internalBuff, SIZE, OVERWRITABLE) {};

    private:
        uint8_t m_internalBuff[SIZE];
    };

}

#endif
