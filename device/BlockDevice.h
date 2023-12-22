/*******************************************************************************
*
*  Name: BlockDevice.h
*
*  Purpose: Contains interface for a block device, where data is accessible in
*           fixed size blocks.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"
#include "return.h"

/// @brief device accessible by reading/writing to/from blocks
class BlockDevice : public Device {
public:
    /// @brief constructor
    BlockDevice(const char* name) : Device(name) {};

    /**
     * @brief Write to a block
     * @param block     Block to write to
     * @param data      Buffer of data, at least get_block_size()
     */
    virtual RetType write(size_t block, uint8_t* data) = 0;

    /**
     * @brief Read from a block
     * @param block     Block number to read from
     * @param buff      Buffer to read into, at least BLOCK_SIZE bytes
     */
    virtual RetType read(size_t block, uint8_t* buff) = 0;

    /**
     * @brief Lock the device, persistently preventing all write and erase_region instructions
     */
    virtual RetType lock() = 0;

    /**
     * @brief Unlock the device, persistently allowing write and erase_region instructions
     */
    virtual RetType unlock() = 0;

    /**
     * @brief Erase the entire device
     */
    virtual RetType clear() = 0;

    /**
     * @return the block size in bytes
     */
    virtual size_t get_block_size() = 0;

    /**
     * @return number of blocks on the device
     */
    virtual size_t get_num_blocks() = 0;
};

#endif
