#ifndef RINGBUFF_H
#define RINGBUFF_H

/// @brief ring buffer
/// @tparam SIZE            the size in bytes of the buffer
/// @tparam OVERWRITABLE    if the buffer is overwritable,
///                         e.g. writing past head moves the head over old data
template <const size_t SIZE, const bool OVERWRITABLE>
class RingBuffer {
public:
    /// @brief constructor
    RingBuffer() : m_head(0), m_tail(0), m_len(0) {};

    /// @brief push data into the buffer
    /// @param buff     the data to push
    /// @param len      the number of bytes to push
    /// @return the number of bytes pushed
    /// Will push at most 'len' bytes onto the buffer
    /// if overwrite is enabled, will guaranteed push 'len' bytes
    size_t push(uint8_t* buff, size_t len) {
        if(m_len + len > SIZE) {
            if(OVERWRITABLE) {
                // move the head back to make room
                m_head = (len + m_tail) % SIZE;
            } else {
                // copy less data
                len = SIZE - m_len;
                m_len += len;
            }
        }

        size_t i = 0;
        while(i < len) {
            m_buff[m_tail] = buff[i];
            i++;
            m_tail = (m_tail + 1) % SIZE;
        }

        return i;
    }

    /// @brief pop data off of the buffer
    /// @param buff     the buffer to copy in to
    /// @param len      the size of the buffer in bytes
    /// @return the actual number of bytes popped
    /// Will pop at most 'len' bytes, but will pop less if there is less data
    /// in the buffer
    size_t pop(uint8_t* buff, size_t len) {
        size_t i = 0;
        while(i < len) {
            buff[i] = m_buff[m_head];
            i++;
            m_head = (m_head + 1) % SIZE;
            if(m_head == m_tail) {
                break;
            }
        }

        m_len -= i;
        return i;
    }

    /// @brief get the current size of the buffer
    /// @return the size of the buffer in bytes
    size_t size() {
        return m_len;
    }

private:
    size_t m_head;
    size_t m_tail;
    uint8_t m_buff[SIZE];
    size_t m_len;
};

#endif
