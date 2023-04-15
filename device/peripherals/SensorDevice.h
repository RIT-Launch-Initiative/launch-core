// TODO: Define an interface for a sensor device

// TODO: For now just have encode method and decode method for their data

// encode(void* sensor_struct, uint8_t buffer)
// Cast void* to whatever the sensor_struct is

// decode(void* sensor_struct, uint8_t buffer)

#ifndef SENSOR_DEVICE_H
#define SENSOR_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

class SensorEncodeDecode{
    virtual void encode(void* sensor_struct, uint8_t buffer) = 0;

    virtual void decode (void* sensor_struct, uint8_t buffer) = 0;
}