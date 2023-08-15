/**
* Test conversion functions
* All tests will be ran at compile time
*
* @author Aaron Chan
*/

#include "common/utils/conversion.h"
#include <cstdint>
#include <cstdio>

constexpr bool test_uint16_to_uint8() {
    uint16_t x = 65535;
    uint8_t buff[2] = {};
    uint16_to_uint8(x, buff);
    return buff[0] == 255 && buff[1] == 255;
}

constexpr bool test_uint32_to_uint8() {
    uint32_t x = 4294967295;
    uint8_t buff[4] = {};
    uint32_to_uint8(x, buff);
    return buff[0] == 255 && buff[1] == 255 && buff[2] == 255 && buff[3] == 255;
}

constexpr bool test_uint64_to_uint8() {
    uint64_t x = 18446744073709551615;
    uint8_t buff[8] = {};
    uint64_to_uint8(x, buff);
    return buff[0] == 255 && buff[1] == 255 && buff[2] == 255 && buff[3] == 255 &&
            buff[4] == 255 && buff[5] == 255 && buff[6] == 255 && buff[7] == 255;
}

constexpr bool test_int16_to_uint8() {
    int16_t x = -1;
    uint8_t buff[2] = {};
    int16_to_uint8(x, buff);
    return buff[0] == 255 && buff[1] == 255;
}

constexpr bool test_int32_to_uint8() {
    int32_t x = -1;
    uint8_t buff[4] = {};
    int32_to_uint8(x, buff);
    return buff[0] == 255 && buff[1] == 255 && buff[2] == 255 && buff[3] == 255;
}

constexpr bool test_int64_to_uint8() {
    int64_t x = -1;
    uint8_t buff[8] = {};
    int64_to_uint8(x, buff);
    return buff[0] == 255 && buff[1] == 255 && buff[2] == 255 && buff[3] == 255 &&
            buff[4] == 255 && buff[5] == 255 && buff[6] == 255 && buff[7] == 255;
}

constexpr bool test_uint8_to_uint16() {
    uint8_t buff[2] = {255, 255};
    uint16_t x = uint8_to_uint16(buff);
    return x == 65535;
}

constexpr bool test_uint8_to_uint32() {
    uint8_t buff[4] = {255, 255, 255, 255};
    uint32_t x = uint8_to_uint32(buff);
    return x == 4294967295;
}

constexpr bool test_uint8_to_uint64() {
    uint8_t buff[8] = {255, 255, 255, 255, 255, 255, 255, 255};
    uint64_t x = uint8_to_uint64(buff);
    return x == 18446744073709551615;
}

constexpr bool test_uint8_to_int16() {
    uint8_t buff[2] = {255, 255};
    int16_t x = uint8_to_int16(buff);
    return x == -1;
}

constexpr bool test_uint8_to_int32() {
    uint8_t buff[4] = {255, 255, 255, 255};
    int32_t x = uint8_to_int32(buff);
    return x == -1;
}

constexpr bool test_uint8_to_int64() {
    uint8_t buff[8] = {255, 255, 255, 255, 255, 255, 255, 255};
    int64_t x = uint8_to_int64(buff);
    return x == -1;
}

constexpr bool test_reconvert_uint16_to_uint8() {
    uint16_t x = 256 * 2;
    uint8_t buff[2] = {};

    uint16_to_uint8(x, buff);
    uint16_t y = uint8_to_uint16(buff);

    return x == y;
}

constexpr bool test_reconvert_uint32_to_uint8() {
    uint32_t x = 65535 * 2;
    uint8_t buff[4] = {};

    uint32_to_uint8(x, buff);
    uint32_t y = uint8_to_uint32(buff);

    return x == y;
}

constexpr bool test_reconvert_uint64_to_uint8() {
    uint64_t x = 4294967295 * 2;
    uint8_t buff[8] = {};

    uint64_to_uint8(x, buff);
    uint64_t y = uint8_to_uint64(buff);

    return x == y;
}

int main() {
    static_assert(test_uint16_to_uint8(), "test_uint16_to_uint8 failed");
    static_assert(test_uint32_to_uint8(), "test_uint32_to_uint8 failed");
    static_assert(test_uint64_to_uint8(), "test_uint64_to_uint8 failed");

    static_assert(test_int16_to_uint8(), "test_int16_to_uint8 failed");
    static_assert(test_int32_to_uint8(), "test_int32_to_uint8 failed");
    static_assert(test_int64_to_uint8(), "test_int64_to_uint8 failed");

    static_assert(test_uint8_to_uint16(), "test_uint8_to_uint16 failed");
    static_assert(test_uint8_to_uint32(), "test_uint8_to_uint32 failed");
    static_assert(test_uint8_to_uint64(), "test_uint8_to_uint64 failed");

    static_assert(test_uint8_to_int16(), "test_uint8_to_int16 failed");
    static_assert(test_uint8_to_int32(), "test_uint8_to_int32 failed");
    static_assert(test_uint8_to_int64(), "test_uint8_to_int64 failed");

    static_assert(test_reconvert_uint16_to_uint8(), "test_reconvert_uint16_to_uint8 failed");
    static_assert(test_reconvert_uint32_to_uint8(), "test_reconvert_uint32_to_uint8 failed");
    static_assert(test_reconvert_uint64_to_uint8(), "test_reconvert_uint64_to_uint8 failed");

    printf("All tests passed!\n");

    return 0;
}






