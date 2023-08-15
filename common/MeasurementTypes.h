/**
 * Structures for standardizing measurements
 *
 * @author Aaron Chan
 */

#ifndef MEASUREMENT_TYPES_H
#define MEASUREMENT_TYPES_H

#include <stdint.h>

typedef struct {
    const uint16_t id; // Unique ID for the sensor
    uint32_t time; // Time of the measurement (unit/format user defined)
} MeasurementInfo;

typedef struct {
    MeasurementInfo info;
    int32_t x_accel;
    int32_t y_accel;
    int32_t z_accel;
} AccelerometerData;

typedef struct {
    MeasurementInfo info;
    int16_t temperature;
    int32_t pressure;
    int32_t altitude;
} AltimeterData;

typedef struct {
    MeasurementInfo info;
    int16_t temperature;
    int16_t humidity;
} AmbientData;

typedef struct {
    MeasurementInfo info;
    int16_t temperature;
    int32_t pressure;
} BarometerData;

typedef struct {
    float time; // seconds since midnight
    float latitude; // degrees (N positive)
    float longitude; // degrees (E positive)
    float alt; // meters ASL
    int quality; // fix indicator
    int num_sats; // number of satellites used
} GPSData;

typedef struct {
    MeasurementInfo info;
    int32_t x_gyro;
    int32_t y_gyro;
    int32_t z_gyro;
} GyroscopeData;

typedef struct {
    MeasurementInfo info;
    int32_t x_mag;
    int32_t y_mag;
    int32_t z_mag;
} MagnetometerData;



#endif // MEASUREMENT_TYPES_H
