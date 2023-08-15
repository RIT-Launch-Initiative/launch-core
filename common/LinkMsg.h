/**
 * Wrapper for constructing payloads between flight computers and ground receivers
 *
 * @author Aaron Chan
 */
#ifndef LINK_MSG_H
#define LINK_MSG_H

#include <stdint.h>

class LinkMsg {
public:
    LinkMsg() {};

    // TODO: Design the structure for link messages

private:
    uint8_t data[64];

};


#endif //LINK_MSG_H
