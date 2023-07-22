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

template <const size_t PACKET_SIZE = MIN_PACKET_SIZE - HEADER_SIZE>
class KISSFrame : public ::Packet {
public:
    KISSFrame() : ::Packet(m_internal_buff, PACKET_SIZE, HEADER_SIZE) {
        m_header = allocate_header<KISS_HEADER_T>();
        m_header->begin = FRAME_END;
        m_header->port_and_command = 0x00;

        uint8_t end = FRAME_END;
        push(&end, 1);
    };

    /**
     * @brief Pushes data to m_packet and checks for special characters to escape
     * @param buff - buffer to push
     * @param len - size of buffer
     * @return RetType - Success of operation
     */
    RetType push_data(uint8_t* buff, size_t len) {
        if (RET_SUCCESS != erase_frame_end()) return RET_ERROR;
        if (len > capacity()) return RET_ERROR;
        uint8_t special = FRAME_ESC;

        for (size_t i = 0; i < len; i++) {
            if (FRAME_END == buff[i]) {
                special = FRAME_ESC;
                if (RET_SUCCESS != push(&special, 1)) return RET_ERROR;
            } else if (TRANS_FRAME_END == buff[i]) {
                special = TRANS_FRAME_ESC;
                if (RET_SUCCESS != push(&special, 1)) return RET_ERROR;
            } else if (TRANS_FRAME_ESC == buff[i] && TRANS_FRAME_ESC == buff[i - 1]) {
                return RET_ERROR; // Protocol Violation or Aborted Transmission Signal
            }

            if (RET_SUCCESS != push(&buff[i], 1)) { // TODO: weird segfault. Just allocate a big enough buffer for now
                return RET_ERROR;
            }
        }

        special = FRAME_END;

        return push(&special, 1);
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

private:
    KISS_HEADER_T* m_header;
    uint8_t m_internal_buff[PACKET_SIZE];

    /**
     * @brief Checks if last byte in m_packet is FRAME_END and erases it if it is
     *
     * @return RetType - Success of operation
     */
    RetType erase_frame_end() {

        if (FRAME_END == *(write_ptr<uint8_t>() - 1)) {
            return erase(1);
        }

        return RET_SUCCESS;
    }
};
}


#endif //LAUNCH_CORE_KISS_H
