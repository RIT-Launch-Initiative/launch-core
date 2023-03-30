#include <sched/macros.h>
#include <stdio.h>
#include <unistd.h>

uint32_t tick = 0;
uint32_t systime() {
    return tick;
}

RetType task2(void*) {
    RESUME();
    printf("task2\n");
    SLEEP(2);
    RESET();
    return RET_SUCCESS;
}

RetType task(void*) {
    RESUME();
    printf("task\n");
    SLEEP(3);
    sched_start(&task2, NULL);
    RESET();
    return RET_ERROR;
}

int main() {
    if(!sched_init(&systime)) {
        printf("failed to init scheduler\n");
        return -1;
    }

    if(sched_start(&task, NULL) < 0) {
        printf("failed to start task\n");
        return -1;
    }

    while(1) {
        printf("tick %u --- ", tick);
        sched_dispatch();
        WAKE(0);
        printf("\n");
        tick++;
        sleep(1);
    }
}
