#ifndef RETURN_H
#define RETURN_H

// return codes

typedef enum {
    RET_SUCCESS = 0,        // success
    RET_ERROR,              // some error
    RET_BLOCKED,            // something blocked
    RET_SLEEP,              // something slept
    RET_YIELD               // something yielded
} RetType;

// if RET_BLOCKED, RET_SLEEP, or RET_YIELD are ever returned, the task must return as fast
// as possible because something called 'sched_block', 'sched_sleep', or wants to return

// only return either code if you called one of those functions and want to
// return control to the scheduler as fast as possible

#endif
