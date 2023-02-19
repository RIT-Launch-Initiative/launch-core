#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "sched/sched.h"
#include "return.h"

uint32_t tick = 0;
uint32_t systime() {
    return tick;
}

RetType task1(void*) {
    printf("task1\n");
    sched_sleep(sched_dispatched, 5);
}

RetType task2(void*) {
    printf("task2\n");
    sched_sleep(sched_dispatched, 1);
}

int main() {
    if(!sched_init(&systime)) {
        printf("failed to init scheduler\n");
        return -1;
    }

    if(sched_start(&task1, NULL) < 0) {
        printf("failed to start task1\n");
        return -1;
    }

    if(sched_start(&task2, NULL) < 0) {
        printf("failed to start task2\n");
        return -1;
    }

    while(1) {
        printf("tick %u --- ", tick);
        sched_dispatch();
        printf("\n");
        tick++;
        sleep(1);
    }
}
