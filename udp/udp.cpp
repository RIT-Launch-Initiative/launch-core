//
// Created by aaron on 9/24/22.
//

#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H

#include <cstdint>
#include "../string/string.h"



class UDP {
//    typedef struct {
//        String src,
//                String dst,
//        uint8_t checksum;
//        String data;
//    } udp_packet_t;


    UDP(String src_addr) : src_addr(src_addr) {}



    int send(String dst, String message) {


        return 0;
    }

private:
    String src_addr;

    int checksum(uint8_t byte_arr[]) {

        return 0;
    }

};


#endif //LAUNCH_CORE_UDP_H
