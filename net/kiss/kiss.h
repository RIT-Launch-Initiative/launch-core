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
#include <cstdio>


namespace kiss {
    typedef enum {
        FRAME_END = 0xC0,
        FRAME_ESC = 0xDB,
        TRANS_FRAME_END = 0xDC,
        TRANS_FRAME_ESC = 0xDD
    } SPECIAL_CHARS_T;

    typedef enum {
        TX_DELAY_CMD = 0x01,
        PERSISTENCE_CMD = 0x02,
        SLOT_TIME_CMD = 0x03,
        TX_TAIL_CMD = 0x04,
        FULL_DUPLEX_CMD = 0x05,
        SET_HW_CMD = 0x06,
        RETURN_CMD = 0xFF
    } COMMANDS_T;

    typedef struct {
        uint8_t begin;
        uint8_t port_and_command;
    } KISS_HEADER_T;

    const uint8_t HEADER_SIZE = sizeof(KISS_HEADER_T);
    const size_t MIN_PACKET_SIZE = 1024;

template <const size_t PACKET_SIZE = MIN_PACKET_SIZE, const size_t PACKET_HEADER_SIZE = HEADER_SIZE>
class KISSFrame {
public:
    KISSFrame() {
        m_header = m_packet.allocate_header<KISS_HEADER_T>();
        m_header->begin = FRAME_END;
        m_header->port_and_command = 0x00;

        push(NULL, 0);
    };

    /**
     * @brief Pushes data to m_packet and checks for special characters to escape
     * @param buff - buffer to push
     * @param len - size of buffer
     * @return RetType - Success of operation
     */
    RetType push(uint8_t* buff, size_t len) {
        erase_frame_end();
        uint8_t special = FRAME_ESC;

        for (size_t i = 0; i < len; i++) {
            if (FRAME_END == buff[i]) {
                special = FRAME_ESC;
                if (RET_SUCCESS != m_packet.push<uint8_t>(special)) return RET_ERROR;
            } else if (TRANS_FRAME_END == buff[i]) {
                special = TRANS_FRAME_ESC;
                if (RET_SUCCESS != m_packet.push<uint8_t>(special)) return RET_ERROR;
            } else if (TRANS_FRAME_ESC == buff[i] && TRANS_FRAME_ESC == buff[i - 1]) {
                return RET_ERROR; // Protocol Violation or Aborted Transmission Signal
            }

            m_packet.push(&buff[i], 1);
        }

        special = FRAME_END;
        return m_packet.push<uint8_t>(special);
    }

    /**
     * @brief Sets the port index of the m_packet (high nibble of command byte)
     * @param port
     */
    void set_port(const uint8_t port) {
        m_header->port_and_command |= port << 4;
    }

    /**
     * @brief Sets the command of the m_packet (low nibble of command byte)
     * @param port
     */
    void set_command(const uint8_t command) {
        m_header->port_and_command |= command;
    }

    /**
     * @brief Sets both the port and command in the command of the m_packet
     * @param port
     * @param command
     */
    void set_port_and_command(const uint8_t port, const uint8_t command) {
        m_header->port_and_command = port << 4 | command;
    }

    [[nodiscard]] uint8_t *raw() {
        return m_packet.raw();
    }

private:
    KISS_HEADER_T* m_header;
    Packet m_packet = alloc::Packet<PACKET_SIZE, PACKET_HEADER_SIZE>();

    /**
     * Checks if last byte in m_packet is FRAME_END and erases it if it is
     */
    void erase_frame_end() {
        uint8_t *write_ptr = m_packet.write_ptr<uint8_t>();

        if (FRAME_END == *write_ptr) {
            m_packet.erase(1);
        }
    }
};
}


#endif //LAUNCH_CORE_KISS_H
