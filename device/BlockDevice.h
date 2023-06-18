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
     * @return          RET_SUCCESS if block written
     */
    virtual RetType write(size_t block, uint8_t* data) = 0;

    /**
     * @brief Read from a block
     * @param block     Block number to read from
     * @param buff      Buffer to read into, at least BLOCK_SIZE bytes
     * @return          RET_SUCCESS if block read
     */
    virtual RetType read(size_t block, uint8_t* buff) = 0;

    /**
     * @brief Lock the device, persistently preventing all write and erase instructions
     * @return RET_SUCCESS if the device is locked
     */
    virtual RetType lock() = 0;

    /**
     * @brief Unlock the device, persistently allowing write and erase instructions
     * @return RET_SUCCESS if the device is unlocked
     */
    virtual RetType unlock() = 0;

    /**
     * @brief Erase the entire device
     * @return Return value of the first CALL to fail, or RET_SUCCESS
     */
    virtual RetType clear() = 0;

    /**
     * @return the block size in bytes
     */
    virtual size_t get_block_size() = 0;

    /**
     * @return number of blocks
     */
    virtual size_t get_num_blocks() = 0;
};

#endif
