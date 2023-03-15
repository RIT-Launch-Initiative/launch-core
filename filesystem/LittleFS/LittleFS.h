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

#define LFS_MKATTRS(...) \
    (struct lfs_mattr[]){__VA_ARGS__}, \
    sizeof((struct lfs_mattr[]){__VA_ARGS__}) / sizeof(struct lfs_mattr)

#define LFS_MKTAG(type, id, size) \
    (((lfs_tag_t)(type) << 20) | ((lfs_tag_t)(id) << 10) | (lfs_tag_t)(size))

#define LFS_MKTAG_IF(cond, type, id, size) \
    ((cond) ? LFS_MKTAG(type, id, size) : LFS_MKTAG(LFS_FROM_NOOP, 0, 0))

#define LFS_MKTAG_IF_ELSE(cond, type1, id1, size1, type2, id2, size2) \
    ((cond) ? LFS_MKTAG(type1, id1, size1) : LFS_MKTAG(type2, id2, size2))

#define LFS_BLOCK_NULL ((lfs_block_t)-1)
#define LFS_BLOCK_INLINE ((lfs_block_t)-2)

typedef uint32_t lfs_tag_t;
typedef int32_t lfs_stag_t;

struct lfs_mattr {
    lfs_tag_t tag;
    const void *buffer;
};

struct lfs_diskoff {
    lfs_block_t block;
    lfs_off_t off;
};

class LittleFS : public FileSystem {
public:
    LittleFS(BlockDevice &block_device) : m_block_device(block_device) {}

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
    BlockDevice &m_block_device;

};

#endif //LAUNCH_CORE_LITTLEFS_H
