#include <sched/sched.h>
#include <sched/macros/macros.h>
#include <stdio.h>
#include <unistd.h>

uint32_t tick = 0;
uint32_t systime() {
    return tick;
}

RetType func() {
    RESUME();

    static int i = 0;

    SLEEP(1);
    i++;

    if(i == 5) {
        // base case
        i = 0;
        return RET_SUCCESS;
    }

    printf("func: i = %i\n", i);
    RetType ret = CALL(func()); // recursion!!!

    return ret;
}

RetType task(void*) {
    RESUME();

    CALL(func());

    SLEEP(1);

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

        task_t* dispatch = sched_select();

        if(NULL != dispatch) {
            printf("dispatching %i\n", dispatch->tid);

            sched_dispatched = dispatch->tid;

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
