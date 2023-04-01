#ifndef SCHED_MACROS_YIELD_H
#define SCHED_MACROS_YIELD_H

#include "sched/sched.h"
#include "return.h"
#include "sched/macros/macros.h"

/* The YIELD macro.
*  Called as YIELD()
*  The current task will return execution to the scheduler, yielding the rest of
*  it's time.
*/

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)


#define YIELD2(z)\
        if(sched_jump[sched_dispatched].size == MAX_CALL_DEPTH) {\
            return RET_ERROR;\
        }\
        sched_jump[sched_dispatched].jumps[sched_jump[sched_dispatched].size++] = TOKENPASTE2(&&_yield, z);\
        return RET_YIELD;\
        TOKENPASTE2(_yield, z):\
        sched_jump[sched_dispatched].size--;\

/// @brief yield back to the scheduler
#define YIELD() YIELD2(__COUNTER__)

#endif
