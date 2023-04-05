#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "filesystem/ChainFS/ChainFS.h"
#include "device/platforms/linux/LinuxBlockDevice.h"

using namespace chainfs;

LinuxBlockDevice block{"fake_fs", 64, 10};
ChainFS fs{block};

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

RetType task(void*) {
    RESUME();

    RetType ret;

    ret = CALL(fs.format());

    if(RET_SUCCESS != ret) {
        printf("failed to format filesystem\n");

        RESET();
        return ret;
    }

    ret = CALL(fs.init());

    if(ret != RET_SUCCESS) {
        printf("failed to initialize filesystem\n");

        RESET();
        return ret;
    }

    // do other stuff like open files and the like
    int fd;
    ret = CALL(fs.open("file.txt", &fd));

    if(ret != RET_SUCCESS) {
        printf("failed to open file\n");

        RESET();
        return ret;
    }

    RESET();
    return RET_ERROR; // just to take us off the scheduler
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

    if(-1 == sched_start(&task, NULL)) {
        printf("failed to start task\r\n");
        return -1;
    }

    while(1) {
        sched_dispatch();
        block.poll();
    }
}
