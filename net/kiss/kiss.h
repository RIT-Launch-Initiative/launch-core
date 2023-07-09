#ifndef LAUNCH_CORE_KISS_H
#define LAUNCH_CORE_KISS_H

#include <stdint.h>
#include "net/packet/Packet.h"


namespace kiss {
    typedef enum {
        const uint8_t FRAME_END = 0xC0;
        const uint8_t FRAME_ESC = 0xDB;
        const uint8_T TRANS_FRAME_END = 0xDC;
        const uint8_t TRANS_FRAME_ESC = 0xDD;
    } special_chars_t;

    typedef enum {
        const uint8_t TX_DELAY_CMD = 0x01;
        const uint8_t PERSISTENCE_CMD = 0x02;
        const uint8_t SLOT_TIME_CMD = 0x03;
        const uint8_t TX_TAIL_CMD = 0x04;
        const uint8_t FULL_DUPLEX_CMD = 0x05;
        const uint8_t SET_HW_CMD = 0x06;
        const uint8_t RETURN_CMD = 0xFF;
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
        Packet::


    }


private:
    kiss_header_t* m_header;



};
}


#endif //LAUNCH_CORE_KISS_H
