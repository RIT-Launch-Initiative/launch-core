/**
* Tests to ensure writing and reading data will be correct
 *
 * @author Aaron Chan
*/

// TODO: Include relevant headers here
#include <cstdint>
#include <cassert>
#include <cstdio>
#include <utils/conversion.h>

// TODO: Remove this struct when you have your own and know what to do
typedef struct {
    uint16_t identifier; // TODO: Port will be the identifier https://wiki.rit.edu/display/ritlaunch/Backplane+Telemetry+Port+Mappings
    int16_t some_flight_data;
    uint8_t some_other_flight_data;
} example_struct_t;


// NOTE: *buffer should point to the start of where you want to write the data into
// Make sure you do not overwrite if you are writing multiple structs into the same buffer
// Each struct and encode/decode functions can go into its related class. Struct at the top and static functions at the bottom outside of the class
void example_encode(example_struct_t *telem, uint8_t *buffer) {
    uint16_to_uint8(telem->identifier, buffer);
    int16_to_uint8(telem->some_flight_data, buffer + 2);
    *(buffer + 4) = telem->some_other_flight_data;
}

void example_decode(example_struct_t *telem, uint8_t *buffer) {
    telem->identifier = uint8_to_uint16(buffer);
    telem->some_flight_data = uint8_to_int16(buffer + 2);
    telem->some_other_flight_data = *(buffer + 4);
}

// TODO: Add tests that encode a single struct each
bool encode_example_struct_test() {
    uint8_t buffer[5] = {};
    example_struct_t example_struct = {10000, -10, 200};

    example_encode(&example_struct, buffer);

    return buffer[0] == 39 && buffer[1] == 16 && buffer[2] == 255 && buffer[3] == 246 && buffer[4] == 200;
}

// TODO: Add tests that can decode a uint8_t buffer and convert it into a struct
bool decode_example_struct_test() {
    uint8_t buffer[5] = {39, 16, 255, 246, 200};
    example_struct_t example_struct = {};

    example_decode(&example_struct, buffer);

    return example_struct.identifier == 10000 && example_struct.some_flight_data == -10 &&
           example_struct.some_other_flight_data == 200;
}

// TODO: Tests where you encode a struct and then decode it and compare the values
bool encode_decode_example_struct_test() {
    uint8_t buffer[5] = {};
    example_struct_t expected = {10000, -10, 200};
    example_struct_t actual = {};

    example_encode(&expected, buffer);
    example_decode(&actual, buffer);

    return expected.identifier == actual.identifier &&
           expected.some_flight_data == actual.some_flight_data &&
           expected.some_other_flight_data == actual.some_other_flight_data;
}


int main() {
    assert(encode_example_struct_test());
    assert(decode_example_struct_test());
    assert(encode_decode_example_struct_test());

    printf("All tests passed!\n");

    return 0;
}


