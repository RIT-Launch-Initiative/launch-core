/**
 * Purpose: FAT Based File System
 * Author: Aaron Chan
 */
#ifndef LAUNCH_CORE_FATFS_H
#define LAUNCH_CORE_FATFS_H

#include <string.h>

#include "filesystem/FileSystem.h"
#include "device/BlockDevice.h"
#include "filesystem/FatFS/descriptors.h"
#include "sched/macros.h"
#include "hashmap/hashmap.h"

namespace fatfs {
    static const size_t MAX_BLOCK_SIZE = 2048;

    class FatFS : public FileSystem {
    public:
        /// @brief constructor
        FatFS(BlockDevice& dev) : m_dev(dev),
                                    last_block(0xFFFFFFFF),
                                    free_block(0xFFFFFFFF) {
            m_blockSize = dev.getBlockSize();
            m_numBlocks = dev.getNumBlocks();
        }

        /// @brief initialize
        RetType init() {
            RESUME();
            // TODO

            RetType ret;


            return RET_SUCCESS;
        }

        /// @brief open a file
        ///        Creates a new file if there isn't one already
        /// @param filename     the name of the file
        /// @param new_file     set to true if a new file was created, false otherwise
        /// @return file descriptor, or -1 on error
        int open(const char* filename, bool* new_file = NULL) {
            // TODO:



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
            // TODO
            return RET_SUCCESS;
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

        }
    };


} // namespace chainfs

#endif //LAUNCH_CORE_FATFS_H