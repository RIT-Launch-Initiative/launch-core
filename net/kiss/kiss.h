#ifndef LAUNCH_CORE_KISS_H
#define LAUNCH_CORE_KISS_H

#include <stdint.h>
#include "net/packet/Packet.h"


namespace kiss {
    typedef enum  {
        const uint8_t FRAME_END = 0xC0;
        const uint8_t FRAME_ESC = 0xDB;
        const uint8_T TRANSP_FRAME_END = 0xDC;
        const uint8_t TRANS_FRAME_ESC = 0xDD;
    } special_chars_t;

// At least 1024 byte long packets
class KISS : public alloc::Packet<1024, 2>  {
public:
    KISS() : Packet() {};



private:


};
}


#endif //LAUNCH_CORE_KISS_H
