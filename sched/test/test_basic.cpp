#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "sched/sched.h"

uint32_t tick = 0;
uint32_t systime() {
    return tick;
}

void task1() {
    printf("task1\n");
    sched_sleep(sched_dispatched, 5);
}

void task2() {
    printf("task2\n");
    sched_sleep(sched_dispatched, 1);
}

int main() {
    if(!sched_init(&systime)) {
        printf("failed to init scheduler\n");
        return -1;
    }

    if(sched_start(&task1) < 0) {
        printf("failed to start task1\n");
        return -1;
    }

    if(sched_start(&task2) < 0) {
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
