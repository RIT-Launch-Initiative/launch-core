#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"
#include "return.h"

/// @brief device accessible by reading/writing to/from blocks
class BlockDevice : public Device {
public:
    /// @brief write to a block
    /// @param block    the block number to write to
    /// @param data     the data to write to the block
    ///                 must be a buffer of at least BLOCK_SIZE bytes
    /// @return 'true' if the entire block was written successfully
    virtual RetType write(size_t block, uint8_t* data) = 0;

    /// @brief read from a block
    /// @param block    the block number to read from
    /// @param buff     the buffer to read into
    ///                 must be a buffer of at least BLOCK_SIZE bytes
    /// @return 'true' if the entire block was read successfully
    virtual RetType read(size_t block, uint8_t* buff) = 0;

    /// @brief get the block size of the device
    /// @return the block size of the device
    virtual size_t getBlockSize() = 0;
};

#endif
