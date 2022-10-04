// Example of how to use scheduler macros

#include "sched/macros.h"


// some function that a task can call
RetType func(int x) {
    // setup this function to be able to block and return execution where
    // it was blocked when it gets scheduled again
    // NOTE: if a different task calls this function, it will start executing
    //       in the last place the last task who called this function stopped at.
    //       This is a problem since functions can only be called from one task
    //       now, working on a solution for this (rather large) limitation.
    RESUME();
    // --> execution begins here


    // check some condition
    if(x > 10) {
        // block the executing task (which called this function)
        // blocked until woken up by the WAKE macro called elsewhere
        BLOCK();
        // --> execution returns here after woken
    }


    // sleep the current task for 10 clock ticks
    SLEEP(10);
    // --> execution returns here after sleep has elapsed


    // reset this function
    // next time the function is called, execution will begin from
    // RESUME() again
    RESET();

    // return to the caller
    return RET_SUCCESS;
}


// an example of what a top-level task may look like
RetType task() {
    // setup this task to be able to block and return execution to where it
    // last returned execution to the scheduler
    RESUME();
    // --> execution begins here

    // Any variables that are used between multiple blocking macros need to be
    // static. If the task/function return execution to the scheduler, the
    // local variables are lost and when execution returns they will not be
    // the same.
    static int x = 0;

    // generally tasks will run forever, returning execution to the scheduler
    // when they can
    while(1) {
        // do some work
        x++;

        // sleep for 100 clock ticks
        SLEEP(100);
        // --> execution returns here after sleep has elapsed

        // wake up some task with task ID 9
        // have to know the task ID from some other knowledge
        // execution is not interrupted
        WAKE(9);

        // make a call to a function that may block, yield, or sleep
        // if the function blocks, yields, or sleeps execution will return
        // to the scheduler. Otherwise RET_SUCCESS or RET_ERROR will be returned
        // to 'ret'

        // --> if blocked, slept, or yielded, execution begins here next time
        // the task is scheduled
        RetType ret = CALL(func(11));

        // yield execution back to the scheduler
        // the scheduler is non-preemptive, so if the task does not yield,
        // block, or sleep no other tasks will get to run
        YIELD();
        // --> execution returns here when task is executed again
    }

    // reset this function
    // next time the function is called, execution will begin from
    // RESUME() again
    // most tasks will have a forever-loop before this and it will never get
    // executed
    RESET();

    // return an error
    // anytime RET_ERROR is returned to the scheduler, the task is deleted and
    // will not be scheduled again
    return RET_ERROR;
}
