#ifndef SCHED_MACROS_YIELD_H
#define SCHED_MACROS_YIELD_H

#include "sched/sched.h"
#include "return.h"

/* The YIELD macro.
*  Called as YIELD()
*  The current task will return execution to the scheduler, yielding the rest of
*  it's time.
*/

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)


#define YIELD2(z)\
        _current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_yield, z);\
        return RET_YIELD;\
        TOKENPASTE2(_yield, z):\

/// @brief yield back to the scheduler
#define YIELD() YIELD2(__COUNTER__)

#endif
