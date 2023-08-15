/**
* Tests to ensure writing and reading data will be correct
 *
 * @author Aaron Chan
*/

// TODO: Include relevant headers here
#include <cstdint>
#include <cassert>
#include <cstdio>
#include "common/utils/conversion.h"

// TODO: Remove this struct when you have your own and know what to do
//typedef struct {
//    uint16_t identifier; // TODO: Port will be the identifier https://wiki.rit.edu/display/ritlaunch/Backplane+Telemetry+Port+Mappings
//    int16_t some_flight_data;
//    uint8_t some_other_flight_data;
//} example_struct_t;


// NOTE: *buffer should point to the start of where you want to write the data into
// Make sure you do not overwrite if you are writing multiple structs into the same buffer
// Each struct and encode/decode functions can go into its related class. Struct at the top and static functions at the bottom outside of the class
// void example_encode(example_struct_t *telem, uint8_t *buffer) {
//     uint16_to_uint8(telem->identifier, buffer);
//     int16_to_uint8(telem->some_flight_data, buffer + 2);
//     *(buffer + 4) = telem->some_other_flight_data;
// }

// void example_decode(example_struct_t *telem, uint8_t *buffer) {
//     telem->identifier = uint8_to_uint16(buffer);
//     telem->some_flight_data = uint8_to_int16(buffer + 2);
//     telem->some_other_flight_data = *(buffer + 4);
// }

void decode(struct *data, uint8_t *buffer){
    
    data->id = uint8_to_int16(buffer);
    switch (data->id)
    {
        // MS5607
        case 10076:
                
            break;

        // BMP3XX
        case 10077:
            data->pressure = uint8_to_int64(buffer+2);
            data->temp = uint8_to_int64(buffer+10);
            data->altitude = uint8_to_int64(buffer + 18);
            break;

        // LSM6
        case 11106:
            data->accX = uint8_to_uint32(buffer + 2);
            data->accY = uint8_to_uint32(buffer + 6);
            data->accZ = uint8_to_uint32(buffer + 10);
            data->gyroX = uint8_to_uint32(buffer + 14);
            data->gyroY = uint8_to_uint32(buffer + 18);
            data->gyroZ = uint8_to_uint32(buffer + 22);
            break;

        // ADXL375
        case 12053:
            data->x_axis = uint8_to_uint32(buffer + 2);
            data->y_axis = uint8_to_uint32(buffer + 6);
            data->z_axis = uint8_to_uint32(buffer + 10);
            break;

        // LIS3
        case 15028:
            data->magX = uint8_to_int64(buffer + 2);
            data->magY = uint8_to_int64(buffer + 6);
            data->magZ = uint8_to_int64(buffer + 10);
            data->temp = uint8_to_int64(buffer + 14);
            break;

        // SHTC3
        case 16070:
            data->temp = uint8_to_int64(buffer + 2);
            data->humidity = uint8_to_int64(buffer + 6);
            break;

        // TMP117
        case 16048:
            data->temp = uint8_to_int64(buffer + 2);
            break;

        default:
            break;
    }
}

void encode(struct *data, uint8_t *buffer) {
    uint16_t id = data->id; 
    switch(id){
        // MS5607
        case 10076:
            uint16_to_uint8(data->id, buffer);
            break;

        //BMP3XX
        case 10077:
            uint16_to_uint8(data->id, buffer);
            uint64_to_uint8(data->pressure, buffer + 2);
            uint64_to_uint8(data->temp, buffer + 10);
            uint64_to_uint8(data->altitude, buffer + 18);
            break;

        // LSM6
        case 11106:
            uint16_to_uint8(data->id, buffer);
            uint32_to_uint8(data->accX, buffer + 2 );
            uint32_to_uint8(data->accY, buffer + 6 );
            uint32_to_uint8(data->accZ, buffer + 10 );
            uint32_to_uint8(data->gyroX, buffer + 14 );
            uint32_to_uint8(data->gyroy, buffer + 18 );
            uint32_to_uint8(data->gyroZ, buffer + 22 );
            break;

        // ADXL375
        case 12053:
            uint16_to_uint8(data->id, buffer);
            uint32_to_uint8(data->x_axis, buffer + 2);
            uint32_to_uint8(data->y_axis, buffer + 6);
            uint32_to_uint8(data->z_axis, buffer + 10);
            break;

        // LIS3
        case 15028:
            uint16_to_uint8(data->id, buffer);
            int32_to_uint8(data->magX, buffer + 2);
            int32_to_uint8(data->magY, buffer + 6);
            int32_to_uint8(data->magZ, buffer + 10);
            int32_to_uint8(data->temp, buffer + 14);
            break;

        // SHTC3
        case 16070:
            uint16_to_uint8(data->id, buffer);
            int32_to_uint8(data->temp, buffer + 2);
            int32_to_uint8(data->humidity, buffer + 6);
            break;

        // TMP117
        case 16048:
            uint16_to_uint8(data->id, buffer);
            int32_to_uint8(data->temp, buffer + 2);
            break;
            
        default:
            break;
    }
     
}


// TODO: Add tests that encode a single struct each
bool encode_example_struct_test() {
    uint8_t buffer[5] = {};
    example_struct_t example_struct = {10000, -10, 200};

    example_encode(&example_struct, buffer);

    return buffer[0] == 39 && buffer[1] == 16 && buffer[2] == 255 && buffer[3] == 246 && buffer[4] == 200;
}

bool encode_BMP_struct_test(){
    uint8_t buffer[3] = {};
    BMP_Readings BMP = {5.0, 12.0, 1000.0};

    encode(&BMP, &buffer);
    return buffer[0] == uint16_to_uint8(10077) && buffer[1] == uint64_to_uint8(5.0) && uint64_to_uint8(12.0) && uint64_to_uint8(1000.0);
 }

bool encode_ADXL_struct_test(){
    uint8_t buffer[26] = {};
    BMP_Readings BMP = {-12, 12.0, 10};

    encode(&BMP, &buffer);
    return buffer[0] == uint16_to_uint8(12053) && buffer[1] == uint16_to_uint8(5);
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


