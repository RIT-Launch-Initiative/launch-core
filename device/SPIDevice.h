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

    /**
     * @brief write and read from the device
     * @param write_buff   the buffer to write
     * @param write_len    the size of 'write_buff' in bytes
     * @param read_buff    the buffer to read into
     * @param read_len     the number of bytes to read
     * @return if all bytes were read successfully
     */
    virtual RetType write_read(uint8_t* write_buff, size_t write_len, uint8_t* read_buff, size_t read_len) = 0;
};

#endif
