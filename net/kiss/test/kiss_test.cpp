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

    uint8_t *test_data = (uint8_t *) "Hello World";

    if (RET_SUCCESS != kiss_packet.push(test_data, strnlen((char *) test_data, 11))) {
        std::cout << "Failed test_push_test_packet: Failed to push" << std::endl;
        return false;
    }

    if (strncmp((char *) kiss_packet.raw() + 2, (char *) test_data, 11) != 0) {
        std::cout << "Failed test_push_test_packet: Mismatched data" << std::endl;
        std::cout << "\tExpected: " << test_data << std::endl;
        std::cout << "\tActual: " << (char *) kiss_packet.raw() + 2 << std::endl;
        return false;
    }

    int end_frame_idx = strnlen((char *) test_data, 11) + 2;
    if (kiss::FRAME_END != kiss_packet.raw()[end_frame_idx]) {
        std::cout << "Failed test_push_test_packet: No FRAME_END at last part of buffer" << std::endl;
        std::cout << "\tExpected: " << (int) kiss::FRAME_END << std::endl;
        std::cout << "\tActual: " << (int) kiss_packet.raw()[end_frame_idx] << std::endl;
        return false;
    }

    return true;
}

bool test_push_test_packet_with_esc() {
    kiss::KISS kiss_packet = kiss::KISS();

    uint8_t test_data[1] = {kiss::SPECIAL_CHARS_T::FRAME_END};
    uint8_t expected_data[3] = {kiss::SPECIAL_CHARS_T::FRAME_ESC, kiss::FRAME_END, kiss::FRAME_END};
    if (RET_SUCCESS != kiss_packet.push(test_data, 1)) {
        std::cout << "Failed test_push_test_packet_with_esc: Failed to push" << std::endl;
        return false;
    }

    if (strncmp((char *) kiss_packet.raw() + 2, (char *) expected_data, 3) != 0) {
        std::cout << "Failed test_push_test_packet_with_esc: Mismatched data" << std::endl;
        std::cout << "\tExpected: " << expected_data << std::endl;
        std::cout << "\tActual: " << (char *) kiss_packet.raw() + 2 << std::endl;
        return false;
    }


    return true;
}

bool test_push_test_packet_with_both_esc() {
    kiss::KISS kiss_packet = kiss::KISS();

    uint8_t test_data[2] = {kiss::SPECIAL_CHARS_T::FRAME_END, kiss::SPECIAL_CHARS_T::TRANS_FRAME_END};
    uint8_t expected_data[5] = {kiss::SPECIAL_CHARS_T::FRAME_ESC, kiss::FRAME_END, kiss::SPECIAL_CHARS_T::TRANS_FRAME_ESC, kiss::TRANS_FRAME_END, kiss::FRAME_END};
    if (RET_SUCCESS != kiss_packet.push(test_data, 2)) {
        std::cout << "Failed test_push_test_packet_with_esc: Failed to push" << std::endl;
        return false;
    }

    if (strncmp((char *) kiss_packet.raw() + 2, (char *) expected_data, 5) != 0) {
        std::cout << "Failed test_push_test_packet_with_esc: Mismatched data" << std::endl;
        std::cout << "\tExpected: " << expected_data << std::endl;
        std::cout << "\tActual: " << (char *) kiss_packet.raw() + 2 << std::endl;
        return false;
    }


    return true;
}

bool test_push_overflow() {
    kiss::KISS kiss_packet = kiss::KISS();

    uint8_t test_data[1024] = {0};

    if (RET_SUCCESS == kiss_packet.push(test_data, 1024)) {
        std::cout << "Failed test_push_overflow: Successfully pushed too big of a packet" << std::endl;
        return false;
    }

    return true;
}

bool test_push_overflow_with_esc() {
    kiss::KISS kiss_packet = kiss::KISS();

    // -3 for the 2 bytes of overhead and 1 byte for the frame end
    uint8_t test_data[1021] = {kiss::SPECIAL_CHARS_T::FRAME_END};
    if (RET_SUCCESS == kiss_packet.push(test_data, 1021)) {
        std::cout << "Failed test_push_overflow_with_esc: Successfully pushed too big of a packet" << std::endl;
        return false;
    }

    return true;
}

bool test_push_consecutive_trans_frame_esc() {
    kiss::KISS kiss_packet = kiss::KISS();

    uint8_t test_data[2] = {kiss::SPECIAL_CHARS_T::TRANS_FRAME_ESC, kiss::SPECIAL_CHARS_T::TRANS_FRAME_ESC};
    if (RET_SUCCESS == kiss_packet.push(test_data, 2)) {
        std::cout << "Failed test_push_consecutive_trans_frame_esc: Did not fail conseuctive TRANS_FRAME_ESC" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    if (!test_no_data()) return -1;
    if (!test_set_port()) return -1;
    if (!test_set_command()) return -1;
    if (!test_set_port_and_command()) return -1;
    if (!test_push_test_packet()) return -1;
    if (!test_push_test_packet_with_esc()) return -1;
    if (!test_push_test_packet_with_both_esc()) return -1;
    if (!test_push_overflow()) return -1;
    if (!test_push_overflow_with_esc()) return -1;
    if (!test_push_consecutive_trans_frame_esc()) return -1;

    std::cout << "All tests passed!" << std::endl;
    return 0;

}