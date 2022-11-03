#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "return.h"

/// @brief network packet
class Packet {
public:
    /// @brief write data to the packet payload
    /// @return
    RetType push(uint8_t* buff, size_t len) {
        if(len + m_wpos > m_size) {
            // no room
            return RET_ERROR;
        }

        memcpy(m_buff + m_wpos, buff, len);
        m_wpos += len;

        return RET_SUCCESS;
    }

    /// @brief write data to the packet payload
    /// @tparam OBJ     type of the object to push
    /// @param obj      the object to push
    /// @return
    template <typename OBJ>
    RetType push(OBJ& obj) {
        return push(reinterpret_cast<uint8_t*>(&obj), sizeof(OBJ));
    }

    /// @brief read data from the packet payload
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

    /// @brief read data from the packet payload
    /// @tparam OBJ     type of object to read data as
    /// @param obj     pointer to the object to read into
    /// @return
    template <typename OBJ>
    RetType read(OBJ* obj) {
        return read(reinterpret_cast<uint8_t*>(obj), sizeof(OBJ));
    }

    /// @brief get the write pointer in the packet
    /// @tparam OBJ     the object type to interpret the data as
    /// @return a pointer to the object, or NULL on error
    template <typename OBJ>
    OBJ* write_ptr() {
        if(m_wpos + sizeof(OBJ) > m_size) {
            // no room
            return NULL;
        }

        return reinterpret_cast<OBJ*>(m_buff + m_wpos);
    }

    /// @brief get the read pointer in the packet
    /// @tparam OBJ     the object type to interpret the data as
    /// @return a pointer to the object, or NULL on error
    template <typename OBJ>
    OBJ* read_ptr() {
        if(m_rpos + sizeof(OBJ) > m_wpos) {
            // no room
            return NULL;
        }

        return reinterpret_cast<OBJ*>(m_buff + m_rpos);
    }

    /// @brief skip reading some bytes in the payload
    /// @param len  the number of bytes to skip
    /// @return
    RetType skip_read(size_t len) {
        if(m_rpos + len > m_wpos) {
            return RET_ERROR;
        }

        m_rpos += len;

        return RET_SUCCESS;
    }

    /// @brief skip writing some bytes in the payload
    /// @param len  the number of bytes to skip
    /// @return
    RetType skip_write(size_t len) {
        if(len + m_wpos > m_size) {
            return RET_ERROR;
        }

        m_wpos += len;

        return RET_SUCCESS;
    }

    /// @brief truncate the packet to only have a certain size remaining
    /// @param size     the new amount of unread data in the packet
    /// @return
    RetType truncate(size_t size) {
        if(size > available()) {
            // can't truncate to bigger than the packet is
            return RET_ERROR;
        }

        // shrink by the difference b/w available and size
        m_rpos -= (available() - size);
    }

    /// @brief allocate a header
    ///        headers are allocated in decreasing memory addresses
    ///         e.g. if the packet is currently:
    ///         | header 2 | header 1 | payload |
    ///         the packet will be:
    ///         | header 3 | header 2 | header 1 | payload
    ///         after calling this function
    /// @tparam HEADER      the type of the header
    /// @return a pointer to the allocated header, or NULL on error
    template <typename HEADER>
    HEADER* allocate_header() {
        if(sizeof(HEADER) > m_hpos) {
            // no space
            return NULL;
        }

        m_hpos -= sizeof(HEADER);
        return reinterpret_cast<HEADER*>(m_buff + m_hpos);
    }

    /// @brief reset the reading position to the beginning of the packet
    /// @param includeHeaders   true if the read position should start at the first header
    ///                         false if the read position should start at the payload
    void seek_read(bool includeHeaders = false) {
        if(includeHeaders) {
            m_rpos = m_hpos;
        } else {
            m_rpos = m_headerSize;
        }
    }

    /// @brief reset the header position to the start of the payload
    void seek_header() {
        m_hpos = m_headerSize;
    }

    /// @brief reset the writing position to the start of the payload
    void seek_write() {
        m_wpos = m_headerSize;
    }

    /// @brief clear the packet
    void clear() {
        m_wpos = m_headerSize;
        m_rpos = m_headerSize;
        m_hpos = m_headerSize;
    }

    /// @brief get how much data is left to read
    /// @return the amount of data available to read, in bytes
    size_t available() {
        return m_wpos - m_rpos;
    }

    /// @brief get how much data is currently written to the packet payload
    /// @return the packets size in bytes
    size_t size() {
        return m_wpos - m_headerSize;
    }

    /// @brief get the how many bytes of header is being used
    /// @return how much header space is used
    size_t headerSize() {
        return m_headerSize - m_hpos;
    }

    /// @brief get the total number of bytes that can be written to the packet payload
    /// @return the capacity of the packet
    size_t capacity() {
        return m_size - m_headerSize;
    }

    /// @brief get the raw pointer to the start of the first header
    /// @return the pointer
    uint8_t* raw() {
        return m_buff + m_hpos;
    }

protected:
    /// @brief protected constructor, use alloc::Packet to declare
    Packet(uint8_t* buff, size_t size, size_t headerSize) :
                                         m_buff(buff), m_size(size + headerSize),
                                         m_headerSize(headerSize),
                                         m_wpos(headerSize),
                                         m_rpos(headerSize),
                                         m_hpos(headerSize) {};

private:
    // buffer
    uint8_t* m_buff;

    // total size of 'm_buff'
    size_t m_size;

    // number of bytes to leave in the beginning for headers
    size_t m_headerSize;

    // write position for the payload
    size_t m_wpos;

    // read position
    size_t m_rpos;

    // position of the first header
    size_t m_hpos;
};

namespace alloc {

/// @brief packet with preallocated space
/// @tparam SIZE            the size in bytes of the packet
/// @tparam HEADERS_SIZE    the number of bytes to preallocate for headers
template <const size_t SIZE, const size_t HEADERS_SIZE>
class Packet : public ::Packet {
public:
    /// @brief constructor
    /// read and write position default to start of payload
    Packet() : ::Packet(m_internalBuff, SIZE, HEADERS_SIZE) {};

private:
    uint8_t m_internalBuff[SIZE + HEADERS_SIZE];
};

}

#endif
