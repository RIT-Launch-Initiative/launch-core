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

    return RET_SUCCESS;
}

RetType func() {
    RESUME();

    static int i = 0;

    YIELD();
    i++;

    if(i == 5) {
        // base case
        // NOTE: fun test, if you remove this line you will likely get a
        //       segfault after i = MAX_CALL_DEPTH - 1
        // i = 0;

        RetType ret = CALL(func2());
        return ret;
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
