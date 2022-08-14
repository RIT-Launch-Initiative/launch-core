#ifndef REGISTER_DEVICE_H
#define REGISTER_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"
#include "return.h"

/// @brief device accessible by reading/writing to/from registers
/// @tparam T   the address type (e.g. uint8_t for I2C)
template <typename T>
class RegisterDevice : Device {
public:
    /// @brief write to the device
    /// @param addr     the register address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return if all bytes were written successfully
    virtual RetType write(T addr, uint8_t* buff, size_t len) = 0;

    /// @brief read from the device
    /// @param addr     the register address to read from
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return if all bytes were read successfully
    virtual RetType read(T addr, uint8_t* buff, size_t len) = 0;
};

#endif
