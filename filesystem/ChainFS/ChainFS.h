/*******************************************************************************
*
*  Name: ChainFS.h
*
*  Purpose: Chain File System
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef CHAIN_FS_H
#define CHAIN_FS_H

#include <string.h>

#include "filesystem/FileSystem.h"
#include "device/BlockDevice.h"
#include "filesystem/ChainFS/descriptors.h"
#include "sched/macros.h"
#include "hashmap/hashmap.h"

namespace chainfs {


static const size_t MAX_BLOCK_SIZE = 2048;

class ChainFS : public FileSystem {
public:
    /// @brief constructor
    ChainFS(BlockDevice& dev) : m_dev(dev),
                                last_block(0xFFFFFFFF),
                                free_block(0xFFFFFFFF) {
        m_blockSize = dev.getBlockSize();
        m_numBlocks = dev.getNumBlocks();
    }

    /// @brief initialize
    RetType init() {
        RESUME();

        RetType ret;

        static descriptor_t* desc;
        static uint32_t block;
        static uint32_t prev_block;
        block = 0;
        prev_block = 0;

        while(block != 0xFFFFFFFF) {
            ret = CALL(m_dev.read(block, m_block));

            if(ret != RET_SUCCESS) {
                return ret;
            }

            desc = (descriptor_t*)m_block;
            prev_block = block;

            block = ((free_descriptor_t*)desc)->header.next_block;

            // store the last free block we see
            if(((free_descriptor_t*)desc)->header.type == FREE_DESCRIPTOR) {
                free_block = prev_block;
                free_desc = *((free_descriptor_t*)desc);
            }

        }

        last_block = prev_block;
        last_desc = *desc;

        return RET_SUCCESS;
    }

    /// @brief open a file
    ///        Creates a new file if there isn't one already
    /// @param filename     the name of the file
    /// @param new_file     set to true if a new file was created, false otherwise
    /// @return file descriptor, or -1 on error
    int open(const char* filename, bool* new_file = NULL) {
        *new_file = false;

        if (m_filenameMap.get(filename) == NULL) {
            free_descriptor_t *fileDesc = m_filenameMap.add(filename);
            if (fileDesc == NULL) {
                return RET_ERROR;
            }

            *fileDesc = this->free_desc;

            m_fileDescMap.add(*fileDesc);

            *new_file = true;
        }



        return RET_SUCCESS;
    }

    /// @brief write to a file
    /// @param fd       descriptor to file to write to
    /// @param buff     data to write
    /// @param len      number of bytes to write
    /// @return
    RetType write(int fd, uint8_t* buff, size_t len) {
        // TODO

        return RET_SUCCESS;
    }

    /// @brief read from a file
    /// @param fd       descriptor to file to read from
    /// @param buff     buffer to read data into
    /// @param len      number of bytes to read
    /// @return
    RetType read(int fd, uint8_t* buff, size_t len) {
        // TODO

        return RET_SUCCESS;
    }

    /// @brief flush a file to persistent storage
    /// @param fd   descriptor to file to flush
    RetType flush(int fd) {
        // TODO

        return RET_SUCCESS;
    }

    /// @brief format the underlying device for this filesystem
    ///        NOTE: destructive operation!!!
    /// @return
    RetType format() {
        RESUME();

        if(m_blockSize > MAX_BLOCK_SIZE) {
            return RET_ERROR;
        }

        if(m_blockSize < sizeof(descriptor_t)) {
            return RET_ERROR;
        }

        // we just write a free descriptor to the first page
        free_descriptor_t desc;
        desc.header = {FREE_DESCRIPTOR, 0xFFFFFFFF};
        desc.start = 1;
        desc.len = m_numBlocks - 1;

        memcpy(m_block, (void*)&desc, sizeof(free_descriptor_t));

        RetType ret = CALL(m_dev.write(0, m_block));

        RESET();
        return ret;
    }

private:
    BlockDevice& m_dev;

    uint8_t m_block[MAX_BLOCK_SIZE];
    size_t m_blockSize;
    size_t m_numBlocks;

    // location and copy of last descriptor in the chain
    uint32_t last_block;
    descriptor_t last_desc;

    // location and copy of currently known free descriptor in the chain
    uint32_t free_block;
    free_descriptor_t free_desc;

    alloc::Hashmap<free_descriptor_t, uint8_t, MAX_BLOCK_SIZE, MAX_BLOCK_SIZE> m_fileDescMap;
    alloc::Hashmap<const char*, free_descriptor_t, MAX_BLOCK_SIZE, MAX_BLOCK_SIZE> m_filenameMap;


    /// @brief helper function to locate free blocks
    RetType get_free_blocks(uint32_t num, uint32_t* start) {
        RESUME();

        RetType ret;

        if(free_block == 0xFFFFFFFF) {
            // there are no free blocks on the system

        }

        while(1) {
            // start at the last free block, if it has enough we're good to go
            if(free_desc.len >= num) {
                // steal blocks from this free chunk
                *start = free_desc.start;
                free_desc.start += num;
                free_desc.len -= num;

                // update the block containing the free descriptor
                memcpy(m_block, (void*)&free_desc, sizeof(free_desc));

                ret = CALL(m_dev.write(free_block, m_block));

                if(ret != RET_SUCCESS) {
                    return ret;
                }

                break;
            }

            // otherwise we need to look for a new free block that's big enough
            // start looking from the beginning of the list
            static free_descriptor_t* desc;
            static uint32_t block;
            block = 0;

            while(block != 0xFFFFFFFF) {
                ret = CALL(m_dev.read(block, m_block));

                if(ret != RET_SUCCESS) {
                    return ret;
                }

                desc = (free_descriptor_t*)m_block;
                if(desc->header.type == FREE_DESCRIPTOR) {
                    if(desc->len >= num) {
                        free_desc = *desc;
                        free_block = block;
                        block = 0;
                        break;
                    }
                }

                // keep looking
                block = desc->header.next_block;
            }

            if(block = 0xFFFFFFFF) {
                // we've looked everywhere and we don't have a chunk big enough :(
                ret = RET_ERROR;
                break;
            }
        }

        RESET();
        return ret;
    }
};


} // namespace chainfs

#endif
