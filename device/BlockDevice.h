#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"

/// @brief device accessible by reading/writing to/from blocks
/// @tparam BLOCK_SIZE the block size in bytes
template <const size_t BLOCK_SIZE>
class BlockDevice : Device {
public:
    /// @brief write to a block
    /// @param block    the block number to write to
    /// @param data     the data to write to the block
    ///                 must be a buffer of at least BLOCK_SIZE bytes
    /// @return 'true' if the entire block was written successfully, 'false' on error
    virtual bool write(size_t block, uint8_t* data) = 0;

    /// @brief read from a block
    /// @param block    the block number to read from
    /// @param buff     the buffer to read into
    ///                 must be a buffer of at least BLOCK_SIZE bytes
    /// @return 'true' if the entire block was read successfully, 'false' on error
    virtual bool write(size_t block, uint8_t* buff) = 0;
};

#endif
