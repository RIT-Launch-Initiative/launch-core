/**
 * @file kiss.h
 *
 * @brief KISS Packet protocol implementation
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_KISS_H
#define LAUNCH_CORE_KISS_H

#include <stdint.h>
#include "net/packet/Packet.h"
#include "return.h"


namespace kiss {
    typedef enum {
        FRAME_END = 0xC0,
        FRAME_ESC = 0xDB,
        TRANS_FRAME_END = 0xDC,
        TRANS_FRAME_ESC = 0xDD
    } special_chars_t;

    typedef enum {
        TX_DELAY_CMD = 0x01,
        PERSISTENCE_CMD = 0x02,
        SLOT_TIME_CMD = 0x03,
        TX_TAIL_CMD = 0x04,
        FULL_DUPLEX_CMD = 0x05,
        SET_HW_CMD = 0x06,
        RETURN_CMD = 0xFF
    } commands_t;

    typedef struct {
        uint8_t begin;
        uint8_t port_and_command;
    } kiss_header_t;



// At least 1024 byte long packets
class KISS : public alloc::Packet<1024, 2>  {
public:
    KISS() : Packet() {
        m_header = Packet::allocate_header<kiss_header_t>();
        m_write_ptr = Packet::write_ptr<uint8_t>();

        m_header->begin = FRAME_END;
        m_header->port_and_command = 0x00;
        *m_write_ptr = FRAME_END;
    };

    /**
     * @brief Pushes data to packet and checks for special characters to escape
     * @param buff - buffer to push
     * @param len - size of buffer
     * @return RetType - Success of operation
     */
    RetType push(uint8_t* buff, size_t len) override {
        uint8_t esc_count = 0;
        uint8_t capacity = Packet::capacity();

        if (len > capacity) return RET_ERROR;
        if (FRAME_END == *(m_write_ptr - 1)) m_write_ptr--;

        for (size_t i = 0; i < len; i++) {
            if (FRAME_END == buff[i]) {
                if (++esc_count + len > capacity) return RET_ERROR;
                *(m_write_ptr++) = TRANS_FRAME_END;
            } else if (FRAME_ESC == buff[i]) {
                if (++esc_count + len > capacity) return RET_ERROR;
                *(m_write_ptr++) = TRANS_FRAME_ESC;
            }

            *(m_write_ptr++) = buff[i];
        }

        *(m_write_ptr++) = FRAME_END;

        return RET_SUCCESS;
    }

    /**
     * @brief Sets the port index of the packet (high nibble of command byte)
     * @param port
     */
    void set_port(uint8_t port) {
        m_header->port_and_command |= port << 4;
    }

    /**
     * @brief Sets the command of the packet (low nibble of command byte)
     * @param port
     */
    void set_command(uint8_t command) {
        m_header->port_and_command |= command;
    }

    /**
     * @brief Sets both the port and command in the command of the packet
     * @param port
     * @param command
     */
    void set_port_and_command(uint8_t port, uint8_t command) {
        m_header->port_and_command = port << 4 | command;
    }

private:
    kiss_header_t* m_header;
    uint8_t *m_write_ptr;
};
}


#endif //LAUNCH_CORE_KISS_H
