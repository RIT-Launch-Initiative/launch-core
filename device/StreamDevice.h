/*******************************************************************************
*
*  Name: StreamDevice.h
*
*  Purpose: Contains the interface for a generic stream device, accessible by
*           reading/writing a dynamic number of bytes.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef STREAM_DEVICE_H
#define STREAM_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"
#include "return.h"

/// @brief device written/read to/from as a stream
class StreamDevice : public Device {
public:
    /// @brief constructor
    StreamDevice(const char* name) : Device(name) {};

    /// @brief write to the stream
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    virtual RetType write(uint8_t* buff, size_t lenz) = 0;

    /// @brief read from the stream
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
    virtual RetType read(uint8_t* buff, size_t len) = 0;

    /// @brief get how many bytes are available for reading without blocking
    /// @return the number of bytes available
    virtual size_t available() = 0;

    /// @brief wait for a certain amount of data to be ready for reading
    ///        blocks the current process, caller must return to scheduler
    ///        as soon as possible
    /// @return RET_BLOCKED or error
    virtual RetType wait(size_t len) = 0;
};

#endif
