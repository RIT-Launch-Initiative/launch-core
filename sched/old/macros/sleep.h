#ifndef SCHED_MACROS_SLEEP_H
#define SCHED_MACROS_SLEEP_H

#include "sched/sched.h"
#include "return.h"
#include "sched/jump_table.h"

/* The SLEEP macro.
*  Called as SLEEP(T)
*  The current task will be put on the sleep queue for T ticks.
*/

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)


#define SLEEP2(N, z)\
            jt_push(sched_dispatched, TOKENPASTE2(&&_sleep, z));\
            sched_jump_table[sched_dispatched].should_jump = 1;\
            sched_sleep(sched_dispatched, N);\
            return RET_YIELD;\
            TOKENPASTE2(_sleep, z):\


/// @brief sleep the currently running task for 'N' ticks
#define SLEEP(N) SLEEP2(N, __COUNTER__)

#endif
