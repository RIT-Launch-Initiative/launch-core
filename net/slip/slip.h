/*******************************************************************************
*
*  Name: slip.h
*
*  Purpose: Implements the Serial Line Internet Protocol (SLIP).
*           While IP is in the name, any protocol can be encapsulated, making
*           SLIP a very simple framing protocol.
*
*  Author: Will Merges
*
*******************************************************************************/
#ifndef SLIP_H
#define SLIP_H

#include <stdlib.h>
#include <stdint.h>

#define SLIP_END     0xC0
#define SLIP_ESC     0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD // double Ds

typedef struct {
    uint8_t* data;
    size_t len;
} slip_buffer_t;

class UnallocatedSLIPEncoder {
public:
    /// @brief encodes data into a SLIP frame
    /// @param data     the data to encode
    /// @param len      the length of 'data' in bytes
    /// @returns a buffer containing the encoded frame, or NULL on error
    slip_buffer_t* encode(uint8_t* data, size_t len) {
        // index in frame.data
        size_t i = 0;

        // start every frame with a frame end
        // while redundant as the last frame should have sent it, we want to make
        // sure this frame is delineated
        //
        // if you really need that one extra byte, don't start with a frame end
        m_buff.data[i++] = SLIP_END;

        // copy the bytes from 'in'
        size_t j;
        for(j = 0; j < len && i < m_size - 1; j++) {
            switch(data[j]) {
                case SLIP_END:
                    m_buff.data[i] = SLIP_ESC;
                    m_buff.data[i + 1] = SLIP_ESC_END;
                    i += 2;
                    break;
                case SLIP_ESC:
                    m_buff.data[i] = SLIP_ESC;
                    m_buff.data[i + 1] = SLIP_ESC_ESC;
                    i += 2;
                    break;
                default:
                    m_buff.data[i++] = data[j];
            }
        }

        if(j < len) {
            // too full :(
            return NULL;
        }

        // if we completely filled the output frame, it's possible we wrote
        // one too many bytes
        //
        // if the last byte in 'in' was SLIP_ESC we squeezed in two bytes when
        // we only had room for one, now we can't fit the frame end
        if(i > 1 && i == m_size - 1) {
            if(m_buff.data[i - 1] == SLIP_ESC) {
                // we couldn't squeeze in the last byte :(
                return NULL;
            }
        } // if i = 1 we've only written the first frame end

        // add the last frame SLIP_END
        m_buff.data[i++] = SLIP_END;
        m_buff.len = i;

        return &m_buff;
    }

    /// @brief encode an object into SLIP frame
    /// @tparam TYPE    the type of object encode
    /// @param data     the data to encode
    /// @return the encoded SLIP frame, or NULL on error
    template <typename TYPE>
    slip_buffer_t* encode(TYPE* data) {
        return encode((uint8_t*)data, sizeof(TYPE));
    }

protected:
    /// @brief protected constructor
    UnallocatedSLIPEncoder(uint8_t* buffer, size_t len) : m_buff{buffer, len},
                                                          m_size(len) {};
private:
    slip_buffer_t m_buff;
    size_t m_size;
};


/// @tparam SIZE    the maximum size of an output SLIP frame
/// NOTE: an output frame can be (N + 1) * 2 bytes, where N is the size of the
///       encapsulated data. The frame is unlikely to be this large, but this is
///       the maximum.
template <size_t SIZE>
class SLIPEncoder : public UnallocatedSLIPEncoder {
public:
    /// @brief constructor
    SLIPEncoder() : ::UnallocatedSLIPEncoder(buffer, SIZE) {};

private:
    uint8_t buffer[SIZE];
};

class UnallocatedSLIPDecoder {
public:
    /// @brief push a byte to be decoded
    /// @param byte     the byte to push
    /// @return a buffer containing a decoded data payload, or NULL if a new
    ///         frame was not completed
    ///
    ///         if a frame is too large to fit, it will be completely discarded
    slip_buffer_t* push(uint8_t byte) {
        // NOTE: we wait for a frame end before parsing a new frame
        //       this could result in missing the first partial frame which is fine
        //
        //       but it also means the first frame encoded should start with a
        //       frame end or it will get ignored
        if(!m_parsing) {
            // waiting for a new SLIP_END byte
            if(SLIP_END == byte) {
                // time to start parsing the frame
                m_escape = false;
                m_parsing = true;

                // reset the buffer
                m_buff.len = 0;

                return NULL;
            }
        }

        // otherwise we're in the middle of parsing a frame
        uint8_t push = byte;

        switch(byte) {
            case SLIP_END:
                // end of the frame
                m_parsing = false;
                return &m_buff;
            case SLIP_ESC:
                m_escape = true;
                return NULL;
            case SLIP_ESC_END:
                if(m_escape) {
                    // the SLIP_END byte was escaped and should be included in the data
                    push = SLIP_END;
                }
            case SLIP_ESC_ESC:
                if(m_escape) {
                    // the SLIP_ESC byte was escaped and should be included in the data
                    push = SLIP_ESC;
                }
            }

        // never an escape if made it here
        m_escape = false;

        // check if we can fit the byte
        if(m_buff.len == m_size) {
            // no more room :(
            m_parsing = false;
            return NULL;
        }

        // push the data byte
        m_buff.data[m_buff.len++] = byte;

        // not a full frame yet
        return NULL;
    }

protected:
    /// @brief protected constructor
    UnallocatedSLIPDecoder(uint8_t* buffer, size_t len) : m_buff{buffer, 0},
                                                          m_size(len),
                                                          m_parsing(false),
                                                          m_escape(false) {};
private:
    slip_buffer_t m_buff;
    size_t m_size;

    bool m_parsing;
    bool m_escape;
};

/// @tparam SIZE    the maximum size of an output data buffer
template <size_t SIZE>
class SLIPDecoder : public UnallocatedSLIPDecoder {
public:
    /// @brief constructor
    SLIPDecoder() : ::UnallocatedSLIPDecoder(buffer, SIZE) {};

private:
    uint8_t buffer[SIZE];
};

#endif
