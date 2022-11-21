#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "filesystem/FatFS/FatFS.h"
#include "device/platforms/linux/LinuxBlockDevice.h"

using namespace littlefs;

LinuxBlockDevice block{"fake_fs", 512, 10};
FatFS fs{block};

uint32_t tick = 0;
uint32_t systime() {
    usleep(1000);

    tick++;
    return tick;
}

RetType task_format() {
    RESUME();

    RetType ret = CALL(fs.format());

    if(RET_SUCCESS != ret) {
        printf("failed to format filesystem\n");
    }

    RESET();
    return ret;
}

RetType task() {
    RESUME();

    RetType ret = CALL(fs.init());

    if(ret != RET_SUCCESS) {
        RESET();
        return ret;
    }

    // do other stuff like open files and the like

    RESET();
    return ret;
}

RetType open() {
    RESUME();

    bool newFileCreated = false;

    int fd = fs.open("test", &newFileCreated);

    if (newFileCreated) {
        printf("New file created successfully");
    } else {
        printf("No file created");
    }

    return RET_SUCCESS;
}

int main() {
    if(RET_SUCCESS != block.init()) {
        printf("Could not init block device\n");
        return -1;
    }

    if(!sched_init(&systime)) {
        printf("failed to initialize scheduler\n");
        return -1;
    }

    if(-1 == sched_start(&task)) {
        printf("failed to start task\r\n");
        return -1;
    }

    if (RET_SUCCESS != open()) {
        printf("Failed to open file");
        return -1;
    }

    while(1) {
        sched_dispatch();
        block.poll();
    }
}