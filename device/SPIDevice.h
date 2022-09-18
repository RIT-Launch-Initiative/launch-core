/*******************************************************************************
*
*  Name: SPIDevice.h
*
*  Purpose: Contains the interface for a SPI contoller device.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"
#include "return.h"

/// @brief SPI controller device
class SPIDevice : public Device {
public:
    /// @brief constructor
    SPIDevice(const char* name) : Device(name) {};

    /// @brief write to the device
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return if all bytes were written successfully
    virtual RetType write(uint8_t* buff, size_t len) = 0;

    /// @brief read from the device
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return if all bytes were read successfully
    virtual RetType read(uint8_t* buff, size_t len) = 0;
};

#endif
