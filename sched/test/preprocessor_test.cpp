// test out scheduler macros with the preprocessor
// this is a good example of what a task would look like
// try compiling with the '-E' flag to see some of the macro magic going on here

#include "sched/macros.h"

// some example task
void task() {
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
    return;
}
