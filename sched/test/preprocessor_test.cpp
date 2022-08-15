// test out scheduler macros with the preprocessor
// this is a good example of what a task would look like
// try compiling with the '-E' flag to see some of the macro magic going on here

#include "sched/macros.h"
#include "return.h"

// some call that a task will make
// e.g. a device call
RetType some_call(int x) {
    RESUME();

    // do some stuff
    int z = 12 + x;

    // block on something
    // this returns RET_BLOCKED
    // if whoever called it uses the CALL macro, they should propogate the blocked
    // return up until the task returns
    BLOCK();

    // we need to reset at the end
    // since we called block, if we didn't do this we would always execute at the block
    // after we get this far once
    RESET();
}

// some example task
RetType task() {
    // resume the task
    // this will jump to wherever SLEEP, BLOCK, or YIELD was called last
    // if not, will start at the top of the function
    // this macro should only be used once per function!
    RESUME();

    while(1) {
        // do tasky things

        // sleep for 10 system ticks
        // this will return the function!
        // task will return here (in the RESUME macro) after scheduled again
        // BE CAREFUL to save variables into statics or onto the tasks stack
        //            (see sched_save/sched_restore for saving onto task stack)
        SLEEP(10);

        // do some more tasky things
        // block on something
        // this will return the function!
        // when someone else calls 'sched_wake' we will resume here
        // again BE CAREFUL about saving variables, this function must be reentrant!
        BLOCK();

        // call some function
        // if we don't get a successful return, give control back to the scheduler
        // someone may have called SLEEP or BLOCK so we need to return as soon as possible
        RetType ret = CALL(some_call(8));

        // wake somebody else up
        // we use a fake TID of 6 for their task ID
        // execution continues past here, does not return!
        WAKE(6);

        // yield our time back to the scheduler
        // this will return the function!
        // BE CAREFUL once again about variables
        YIELD();
    }


    // we should never get here :(
    RESET();
    return RET_ERROR;
}
