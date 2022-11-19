/**
 * Purpose: Little File System
 * Author: Aaron Chan
 */
#ifndef LAUNCH_CORE_FATFS_H
#define LAUNCH_CORE_FATFS_H

#include <string.h>

#include "filesystem/FileSystem.h"
#include "device/BlockDevice.h"
#include "filesystem/LittleFS/descriptors.h"
#include "sched/macros.h"
#include "hashmap/hashmap.h"

namespace littlefs {
    static const size_t MAX_BLOCK_SIZE = 2048;
    static const MAX_NAME_SIZE = 255;
    static const MAX_FILE_SIZE = 2147483647; // TODO: Determine file size or make it configurable
    static const MAX_ATTRIBUTES = 1022;

    typedef enum {
        LFS_RET_OK          = 0,    // No error
        LFS_RET_IO          = -5,   // Error during device operation
        LFS_RET_CORRUPT     = -84,  // Corrupted
        LFS_RET_NOENT       = -2,   // No directory entry
        LFS_RET_EXIST       = -17,  // Entry already exists
        LFS_RET_NOTDIR      = -20,  // Entry is not a dir
        LFS_RET_ISDIR       = -21,  // Entry is a dir
        LFS_RET_NOTEMPTY    = -39,  // Dir is not empty
        LFS_RET_BADF        = -9,   // Bad file number
        LFS_RET_FBIG        = -27,  // File too large
        LFS_RET_INVAL       = -22,  // Invalid parameter
        LFS_RET_NOSPC       = -28,  // No space left on device
        LFS_RET_NOMEM       = -12,  // No more memory available
        LFS_RET_NOATTR      = -61,  // No data/attr available
        LFS_RET_NAMETOOLONG = -36,  // File name too long
    } LFS_RET;

    class LittleFS : public FileSystem {
    public:
        /// @brief constructor
        LittleFS(BlockDevice& dev) : m_dev(dev),
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
            // TODO


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
            return RET_SUCCESS;

        }

    private:
        BlockDevice& m_dev;

        uint8_t m_block[MAX_BLOCK_SIZE];
        size_t m_blockSize;
        size_t m_numBlocks;

        uint32_t checksum(uint32_t crc, const void *buffer, size_t size) {
            static const uint32_t table[16] = {
                    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
                    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
                    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
                    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
            }

            const uint8_t *data = buffer;

            for (size_t i = 0; i < size; i++) {
                crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
                crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
            }

            return crc;
        }
    };


} // namespace littlefs

#endif //LAUNCH_CORE_LittleFS_H
