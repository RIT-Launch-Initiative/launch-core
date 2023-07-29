/**
 * Wrapper for constructing payloads between flight computers and ground receivers
 *
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_LINKMSG_H
#define LAUNCH_CORE_LINKMSG_H

#include <stdint.h>

class LinkMsg {
public:
    LinkMsg() {};

    // TODO: Design the structure for link messages

private:
    uint8_t data[64];

};


#endif //LAUNCH_CORE_LINKMSG_H
