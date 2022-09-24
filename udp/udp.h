//
// Created by aaron on 9/24/22.
//

#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include "../string/string.h"
#include ""

class UDP {

    UDP(alloc::String<16> src_addr) : src_addr(src_addr) {}

private:
    private alloc::String<16> src_addr;

};


#endif //LAUNCH_CORE_UDP_H
