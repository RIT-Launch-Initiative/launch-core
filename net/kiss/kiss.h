#ifndef LAUNCH_CORE_KISS_H
#define LAUNCH_CORE_KISS_H

#include <stdint.h>
#include "net/packet/Packet.h"


namespace kiss {
    typedef enum {
        FRAME_END = 0xC0;
        FRAME_ESC = 0xDB;
        TRANS_FRAME_END = 0xDC;
        TRANS_FRAME_ESC = 0xDD;
    } special_chars_t;

    typedef enum {
        TX_DELAY_CMD = 0x01;
        PERSISTENCE_CMD = 0x02;
        SLOT_TIME_CMD = 0x03;
        TX_TAIL_CMD = 0x04;
        FULL_DUPLEX_CMD = 0x05;
        SET_HW_CMD = 0x06;
        RETURN_CMD = 0xFF;
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
    };

    void set_port(uint8_t port) {
        m_header->port_and_command |= port << 4;
    }

    void set_command(uint8_t command) {
        m_header->port_and_command |= command;
    }

    void set_port_and_command(uint8_t port, uint8_t command) {
        m_header->port_and_command = port << 4 | command;
    }


private:
    kiss_header_t* m_header;



};
}


#endif //LAUNCH_CORE_KISS_H
