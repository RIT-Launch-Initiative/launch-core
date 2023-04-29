#ifndef SCHED_MACROS_SLEEP_H
#define SCHED_MACROS_SLEEP_H

#include "sched/sched.h"
#include "return.h"
#include "sched/macros/macros.h"

/* The SLEEP macro.
*  Called as SLEEP(T)
*  The current task will be put on the sleep queue for T ticks.
*/

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)


#define SLEEP2(N, z)\
            if(unlikely(sched_jump[sched_dispatched].size == MAX_CALL_DEPTH)) {\
                return RET_ERROR;\
            }\
            sched_jump[sched_dispatched].jumps[sched_jump[sched_dispatched].size++] = TOKENPASTE2(&&_sleep, z);\
            sched_sleep(sched_dispatched, N);\
            return RET_YIELD;\
            TOKENPASTE2(_sleep, z):\
            sched_jump[sched_dispatched].size--;\


/// @brief sleep the currently running task for 'N' ticks
#define SLEEP(N) SLEEP2(N, __COUNTER__)

#endif
