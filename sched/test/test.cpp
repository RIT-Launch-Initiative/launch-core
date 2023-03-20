#include <sched/macros.h>
#include <stdio.h>
#include <unistd.h>

uint32_t tick = 0;
uint32_t systime() {
    return tick;
}

RetType task(void*) {
    RESUME();
    printf("task\n");
    SLEEP(3);
    RESET();
    return RET_SUCCESS;
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
        printf("\n");
        tick++;
        sleep(1);
    }
}
