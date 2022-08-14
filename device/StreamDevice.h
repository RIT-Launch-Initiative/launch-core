#ifndef STREAM_DEVICE_H
#define STREAM_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"

/// @brief device written/read to/from as a stream
class StreamDevice : public Device {
public:
    /// @brief write to the stream
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return the number of bytes written successfully
    virtual size_t write(uint8_t* buff, size_t len) = 0;

    /// @brief read from the stream
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return the number of bytes read successfully
    virtual size_t read(uint8_t* buff, size_t len) = 0;
};

#endif
