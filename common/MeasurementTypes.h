/**
 * Data Structures for standardizing measurements
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_MEASUREMENTTYPES_H
#define LAUNCH_CORE_MEASUREMENTTYPES_H

#include <stdint.h>

typedef struct {
    const uint16_t id; // Unique ID for the sensor
    uint32_t time; // Time of the measurement (unit/format user defined)
} MEASUREMENT_INFO_T;

typedef struct {
    MEASUREMENT_INFO_T info;
    int32_t x_accel;
    int32_t y_accel;
    int32_t z_accel;
} ACCEL_DATA_T;

typedef struct {
    MEASUREMENT_INFO_T info;
    int16_t temperature;
    int32_t pressure;
    int32_t altitude;
} ALTIMETER_DATA_T;

typedef struct {
    MEASUREMENT_INFO_T info;
    int16_t temperature;
    int16_t humidity;
} AMBIENT_DATA_T;

typedef struct {
    MEASUREMENT_INFO_T info;
    int16_t temperature;
    int32_t pressure;
} BAROMETER_DATA_T;

typedef struct {
    float time; // seconds since midnight
    float latitude; // degrees (N positive)
    float longitude; // degrees (E positive)
    float alt; // meters ASL
    int quality; // fix indicator
    int num_sats; // number of satellites used
} GGA_DATA_T;

typedef struct {
    MEASUREMENT_INFO_T info;
    int32_t x_gyro;
    int32_t y_gyro;
    int32_t z_gyro;
} GYRO_DATA_T;

typedef struct {
    MEASUREMENT_INFO_T info;
    int32_t x_mag;
    int32_t y_mag;
    int32_t z_mag;
} MAG_DATA_T;



#endif //LAUNCH_CORE_MEASUREMENTTYPES_H
