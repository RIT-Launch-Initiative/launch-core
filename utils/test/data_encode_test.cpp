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
//typedef struct {
//    uint16_t identifier; // TODO: Port will be the identifier https://wiki.rit.edu/display/ritlaunch/Backplane+Telemetry+Port+Mappings
//    int16_t some_flight_data;
//    uint8_t some_other_flight_data;
//} example_struct_t;

typedef struct ADXL_Readings
{
    uint16_t id;
    uint32_t x_axis;
    uint32_t y_axis;
    uint32_t z_axis;
} ADXL_Readings_default = {12053,null,null,null};

typedef struct BMP_Readings
{
    uint16_t id;
    double pressure;
    double temp;
    double altitude;
} BMP_Readings_default = {10077,null,null,null};

typedef struct TMP_Readings
{
    uint16_t id;
    float temp;
} TMP_Readings_default = {16048,null};

typedef struct LSM_Readings
{
    uint16_t id;
    uint32_t accX;
    uint32_t accY;
    uint32_t accZ;
    uint32_t gyroX;
    uint32_t gyroY;
    uint32_t gyroZ;
} LSM_Readings_default = {11106,null,null,null,null,null,null};


typedef struct LIS_Readings
{
    uint16_t id;
    float magX;
    float magY;
    float magZ;
    float temp;
} LIS_Readings_default = {15028,null,null,null,null};

typedef struct MS_Reading
{
    uint16_t id;
    float pressure;
    float temp;
} MS_Readings_default = {10076,null,null};

typedef struct SHTC_Readings
{
    uint16_t id;
    float temp;
    float humidity;
} SHTC_Readings_default = {16070,null,null};



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

void decode(struct *data, uint8_t *buffer){
    
    data->id = uint8_to_int16(buffer);
    switch (data->id)
    {
    case 10076:
            
            break;
        case 10077:
            data->pressure = uint8_to_int64(buffer+2);
            data->temp = uint8_to_int64(buffer+10);
            data->altitude = uint8_to_int64(buffer + 18);
            break;
        case 11106:
            data->accX = uint8_to_int32(buffer + 2);
            data->accY = uint8_to_int32(buffer + 6);
            data->accZ = uint8_to_int32(buffer + 10);
            data->gyroX = uint8_to_int32(buffer + 14);
            data->gyroY = uint8_to_int32(buffer + 18);
            data->gyroZ = uint32_to_uint8(buffer + 22);
            break;
        case 12053:
            data->x_axis = uint32_to_uint8(buffer + 2);
            data->y_axis = uint8_to_int32(buffer + 6);
            data->z_axis = uint32_to_uint8(buffer 10);
            break;
        case 15028:
            
            break;
        case 16070:
            
            break;
        case 16048:
            
            break;
        default:
            break;
    }
}

void encode(struct *data, uint8_t *buffer) {
    uint16_t id = data->id; 
    switch(id){
        case 10076:
            uint16_to_uint8(data->id, buffer);
            break;
        case 10077:
            uint16_to_uint8(data->id, buffer);
            uint64_to_uint8(data->pressure, buffer + 2);
            uint64_to_uint8(data->temp, buffer + 10);
            uint64_to_uint8(data->altitude, buffer + 18);
            break;
        case 11106:
            uint16_to_uint8(data->id, buffer);
            uint32_to_uint8(data->accX, buffer + 2 );
            uint32_to_uint8(data->accY, buffer + 6 );
            uint32_to_uint8(data->accZ, buffer + 10 );
            uint32_to_uint8(data->gyroX, buffer + 14 );
            uint32_to_uint8(data->gyroy, buffer + 18 );
            uint32_to_uint8(data->gyroZ, buffer + 22 );
            break;
        case 12053:
            uint16_to_uint8(data->id, buffer);
            uint32_to_uint8(data->x_axis, buffer + 2);
            uint32_to_uint8(data->y_axis, buffer + 6);
            uint32_to_uint8(data->z_axis, buffer + 10);
            break;
        case 15028:
            uint16_to_uint8(data->id, buffer);
            break;
        case 16070:
            uint16_to_uint8(data->id, buffer);
            break;
        case 16048:
            uint16_to_uint8(data->id, buffer);
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
    uint8_t buffer[26] = {};
    BMP_Readings BMP = {5.0, 12.0, 1000.0};

    encode(&BMP, &buffer);
    return buffer[0] == uint16_to_uint8(10077) && buffer[1] == uint16_to_uint8(5);
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


