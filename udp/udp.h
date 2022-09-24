//
// Created by aaron on 9/24/22.
//

#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include "../string/string.h"
#include ""

class UDP {

    UDP(String src_addr) : src_addr(src_addr) {}

    int checksum(byte[] byte_arr) {

        return 0;
    }

    int send(String dst, String message) {


        return 0;
    }

private:
    private String src_addr;

};


#endif //LAUNCH_CORE_UDP_H
