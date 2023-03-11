/*******************************************************************************
*
*  Name: I2CDevice.h
*
*  Purpose: Contains the interface for an I2C controller device.
*
*  Author: Will Merges
 *         Aaron Chan
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef I2C_DEVICE_H
#define I2C_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"
#include "return.h"

// I2C address
typedef struct {
    uint16_t dev_addr;      // device address
    uint16_t mem_addr;      // memory address
    uint16_t mem_addr_size; // size of memory addresses (in bytes)
} I2CAddr_t;

/// @brief I2C controller device
class I2CDevice : public Device {
public:
    /// @brief constructor
    I2CDevice(const char* name) : Device(name) {};

    /// @brief transmit to the device
    /// @param addr     the register address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @param timeout  the timeout of the operation, or '0' for no timeout
    /// @return if all bytes were written successfully
    virtual RetType transmit(I2CAddr_t &addr, uint8_t *buff,
                             size_t len, uint32_t timeout = 0) = 0;

    /// @brief receive from the device
    /// @param addr     the register address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @param timeout  the timeout of the operation, or '0' for no timeout
    /// @return if all bytes were read successfully
    virtual RetType receive(I2CAddr_t &addr, uint8_t *buff,
                            size_t len, uint32_t timeout = 0) = 0;

    /// @brief write to a specific device register
    /// @param addr     the register address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @param timeout  the timeout of the operation, or '0' for no timeout
    /// @return if all bytes were written successfully
    virtual RetType write(I2CAddr_t& addr, uint8_t* buff,
                          size_t len, uint32_t timeout = 0) = 0;

    /// @brief read from a specific device register
    /// @param addr     the register address to read from
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @param timeout  the timeout of the operation, or '0' for no timeout
    /// @return if all bytes were read successfully
    virtual RetType read(I2CAddr_t& addr, uint8_t* buff,
                         size_t len, uint32_t timeout = 0) = 0;


};

#endif
