/**
* @file kiss_test.cpp
 *
 * @brief KISS Packet protocol implementation test file
 * @author Aaron Chan
*/

// TODO: GTest

#include <stdint.h>
#include <iostream>
#include "net/kiss/kiss.h"

bool test_no_data() {
    kiss::KISS kiss_packet = kiss::KISS();


    uint8_t *buff = kiss_packet.raw();
    if (kiss::FRAME_END != buff[0]) {
        std::cout << "Failed test_no_data: No FRAME_END at index 0" << std::endl;
        return false;
    }

    if (0x00 != buff[1]) {
        std::cout << "Failed test_no_data: No 0x00 at index 1" << std::endl;
        return false;
    }

    if (kiss::FRAME_END != buff[2]) {
        std::cout << "Failed test_no_data: No FRAME_END at index 2" << std::endl;
        return false;
    }

    return true;
}


int main(int argc, char** argv) {
    if (!test_no_data()) return -1;

    std::cout << "All tests passed!" << std::endl;
    return 0;

}