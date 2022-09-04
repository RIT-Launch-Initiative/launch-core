#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "return.h"

/// @brief network packet
class Packet {
public:
    /// @brief write data to the packet
    /// @return
    RetType push(uint8_t* buff, size_t len) {
        if(len + m_wpos >= m_size) {
            // no room
            return RET_ERROR;
        }

        memcpy(m_buff + m_wpos, buff, len);
        m_wpos += len;

        return RET_SUCCESS;
    }

    /// @brief write data to the packet
    /// @tparam OBJ     type of the object to push
    /// @param obj      the object to push
    /// @return
    template <typename OBJ>
    RetType push(OBJ& obj) {
        return push(reinterpret_cast<uint8_t*>(&obj), sizeof(OBJ));
    }

    /// @brief read data from the packet
    /// @param buff     buffer to read into
    /// @param len      size of 'buff' in bytes
    RetType read(uint8_t* buff, size_t len) {
        if(len + m_rpos > m_wpos) {
            // not enough to read
            return RET_ERROR;
        }

        memcpy(buff, m_buff + m_rpos, len);
        m_rpos += len;

        return RET_SUCCESS;
    }

    /// @brief read data from the packet
    /// @tparam OBJ     type of object to read data as
    /// @param obj     pointer to the object to read into
    /// @return
    template <typename OBJ>
    RetType read(OBJ* obj) {
        return read(reinterpret_cast<uint8_t*>(obj), sizeof(OBJ));
    }

    /// @brief get a pointer to data in the packet
    /// @tparam OBJ     the object type to interpret the data as
    /// @return a pointer to the object, or NULL on error
    template <typename OBJ>
    OBJ* ptr() {
        if(m_wpos + sizeof(OBJ) > m_size) {
            // no room
            return NULL;
        }

        return reinterpret_cast<OBJ*>(raw() + m_wpos);
    }

    /// @brief skip reading some bytes
    /// @param len  the number of bytes to skip
    /// @return
    RetType skip_read(size_t len) {
        if(m_rpos + len > m_wpos) {
            return RET_ERROR;
        }

        m_rpos += len;

        return RET_SUCCESS;
    }

    /// @brief skip writing some bytes
    /// @param len  the number of bytes to skip
    /// @return
    RetType skip_write(size_t len) {
        if(len + m_wpos >= m_size) {
            return RET_ERROR;
        }

        m_wpos += len;

        return RET_SUCCESS;
    }

    /// @brief move the write position back
    /// @param len  the amount to move the the write position back by
    RetType reverse(size_t len) {
        if(len > m_wpos) {
            return RET_ERROR;
        }

        m_wpos -= len;
        return RET_SUCCESS;
    }

    /// @brief reset the reading position to the beginning of the packet
    void seek() {
        m_rpos = 0;
    }

    /// @brief clear the packet
    void clear() {
        m_wpos = 0;
        m_rpos = 0;
    }

    /// @brief get how much data is left to read
    /// @return the amount of data available to read
    size_t available() {
        return m_wpos - m_rpos;
    }

    /// @brief get how much data is currently written to the packet
    /// @return the packets size
    size_t size() {
        return m_wpos;
    }

    /// @brief get the total number of bytes that can be written to the packet
    /// @return the capacity of the packet
    size_t capacity() {
        return m_size;
    }

    /// @brief get the raw pointer to the beginning of the packet payload
    /// @return the pointer
    uint8_t* raw() {
        return m_buff;
    }

protected:
    /// @brief protected constructor, use alloc::Packet to declare
    Packet(uint8_t* buff, size_t size) : m_buff(buff), m_size(size),
                                         m_wpos(0), m_rpos(0) {};

private:
    uint8_t* m_buff;
    size_t m_size;

    // write position
    size_t m_wpos;

    // read position
    size_t m_rpos;
};

namespace alloc {

/// @brief packet with preallocated space
/// @tparam SIZE      the size in bytes of the packet
template <const size_t SIZE>
class Packet : public ::Packet {
public:
    /// @brief constructor
    Packet() : ::Packet(m_internalBuff, SIZE) {};

private:
    uint8_t m_internalBuff[SIZE];
};

}

#endif
