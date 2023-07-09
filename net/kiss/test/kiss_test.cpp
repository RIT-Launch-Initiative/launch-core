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

bool test_set_port() {
    kiss::KISS kiss_packet = kiss::KISS();
    kiss_packet.set_port(1);

    if (0x10 != kiss_packet.raw()[1]) {
        std::cout << "Failed test_set_port: No 0x01 at index 1" << std::endl;
        return false;
    }

    return true;
}

bool test_set_command() {
    kiss::KISS kiss_packet = kiss::KISS();

    kiss_packet.set_command(kiss::TX_DELAY_CMD);
    if (0x01 != kiss_packet.raw()[1]) {
        std::cout << "Failed test_set_command: No 0x01 at index 1" << std::endl;
        return false;
    }

    return true;
}

bool test_set_port_and_command() {
    kiss::KISS kiss_packet = kiss::KISS();

    kiss_packet.set_port_and_command(1, kiss::TX_DELAY_CMD);

    if (0x11 != kiss_packet.raw()[1]) {
        std::cout << "Failed test_set_port_and_command: No 0x11 at index 1" << std::endl;
        return false;
    }

    return true;
}

bool test_push_test_packet() {
    kiss::KISS kiss_packet = kiss::KISS();

    if (RET_SUCCESS != kiss_packet.push((uint8_t*)"Hello World", 11)) {
        std::cout << "Failed test_push_test_packet: Failed to push" << std::endl;
        return false;
    }

    if (strncmp((char*)kiss_packet.raw() + 3, "Hello World", 11) != 0) {
        std::cout << "Failed test_push_test_packet: Mismatched data" << std::endl;
        std::cout << "\tExpected: Hello World" << std::endl;
        std::cout << "\tActual: " << (char *) kiss_packet.raw() + 3 << std::endl;
        return false;
    }

    if (kiss::FRAME_END != kiss_packet.raw()[14]) {
        std::cout << "Failed test_push_test_packet: No FRAME_END at index 14" << std::endl;
        return false;
    }



    return true;
}

bool test_push_test_packet_with_esc() {
    kiss::KISS kiss_packet = kiss::KISS();

    return true;
}

bool test_push_test_packet_with_esc_and_frame_end() {
    kiss::KISS kiss_packet = kiss::KISS();

    return true;
}

bool test_push_overflow() {
    kiss::KISS kiss_packet = kiss::KISS();

    return true;
}

bool test_push_overflow_with_esc() {
    kiss::KISS kiss_packet = kiss::KISS();

    return true;
}

int main(int argc, char** argv) {
    if (!test_no_data()) return -1;
    if (!test_set_port()) return -1;
    if (!test_set_command()) return -1;
    if (!test_set_port_and_command()) return -1;
    if (!test_push_test_packet()) return -1;
    if (!test_push_test_packet_with_esc()) return -1;
    if (!test_push_test_packet_with_esc_and_frame_end()) return -1;
    if (!test_push_overflow()) return -1;
    if (!test_push_overflow_with_esc()) return -1;

    std::cout << "All tests passed!" << std::endl;
    return 0;

}