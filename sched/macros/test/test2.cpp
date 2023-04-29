#include <sched/sched.h>
#include <sched/macros/macros.h>
#include <stdio.h>
#include <unistd.h>

uint32_t tick = 0;
uint32_t systime() {
    return tick;
}

RetType func2() {
    RESUME();

    printf("func2!\n");

    BLOCK();

    return RET_SUCCESS;
}

RetType func() {
    RESUME();

    printf("func1\n");

    CALL(func2());

    return RET_SUCCESS;
}

RetType task1(void*) {
    RESUME();

    CALL(func());

    return RET_SUCCESS;
}

RetType task2(void*) {
    RESUME();

    SLEEP(10);

    WAKE(0);

    return RET_SUCCESS;
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

        task_t* dispatch = sched_select();

        if(NULL != dispatch) {
            printf("dispatching %i\n", dispatch->tid);

            sched_dispatched = dispatch->tid;
            sched_jump[sched_dispatched].index = 0;

            RetType ret = dispatch->func(dispatch->arg);

            if(RET_ERROR == ret) {
                sched_kill(sched_dispatched);
            }
        }

        printf("\n");
        tick++;
        sleep(1);
    }
}
