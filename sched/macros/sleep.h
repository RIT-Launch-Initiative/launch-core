#ifndef SCHED_MACROS_SLEEP_H
#define SCHED_MACROS_SLEEP_H

#include "sched/sched.h"
#include "return.h"

/* The SLEEP macro.
*  Called as SLEEP(T)
*  The current task will be put on the sleep queue for T ticks.
*/

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)


#define SLEEP2(N, z)\
            _current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_sleep, z);\
            sched_sleep(sched_dispatched, N);\
            return RET_SLEEP;\
            TOKENPASTE2(_sleep, z):\

/// @brief sleep the currently running task for 'N' ticks
#define SLEEP(N) SLEEP2(N, __COUNTER__)

#endif
