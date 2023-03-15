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
    BlockDevice* m_block_device;

};


#endif //LAUNCH_CORE_LITTLEFS_H
