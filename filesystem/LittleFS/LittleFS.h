//
// Created by aaron on 3/14/23.
//

#ifndef LAUNCH_CORE_LITTLEFS_H
#define LAUNCH_CORE_LITTLEFS_H


#include "filesystem/FileSystem.h"
#include "device/BlockDevice.h"
#include "lfs.h"
#include "lfs_util.h"
#include "sched/macros.h"

class LittleFS : public FileSystem {
public:
    LittleFS(BlockDevice& block_device) : m_block_device(block_device) {}

    RetType init() {
        RESUME();

        lfs_cfg.context = &m_block_device;
        lfs_cfg.read_size = m_block_device.getBlockSize();
        lfs_cfg.prog_size = m_block_device.getBlockSize();
        lfs_cfg.block_size = m_block_device.getBlockSize();
        lfs_cfg.block_count = m_block_device.getNumBlocks();
        lfs_cfg.block_cycles = 500;
        lfs_cfg.cache_size = 512;
        lfs_cfg.lookahead_size = 512;
        lfs_cfg.name_max = 255;
        lfs_cfg.file_max = 0;
        lfs_cfg.attr_max = 0;

        int err = lfs_mount(&lfs, &lfs_cfg);
        if (err) {
            err = lfs_format(&lfs, &lfs_cfg);
            if (err) {
                RESET();
                return RET_ERROR;
            }

            err = lfs_mount(&lfs, &lfs_cfg);
            if (err) {
                RESET();
                return RET_ERROR;
            }
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType open(const char *filename, int *fd, bool *new_file = NULL) {
        RESUME();



        RESET();
        return RET_SUCCESS;
    }

    RetType write(int fd, uint8_t *buff, size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    RetType read(int fd, uint8_t *buff, size_t len) {
        RESUME();



        RESET();
        return RET_SUCCESS;
    }

    RetType flush(int fd) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    virtual RetType format() {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

private:
    lfs_t lfs;
    lfs_config lfs_cfg;
    BlockDevice& m_block_device;

};


#endif //LAUNCH_CORE_LITTLEFS_H
