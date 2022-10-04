#ifndef SCHED_MACROS_BLOCK_H
#define SCHED_MACROS_BLOCK_H

#include "sched/sched.h"
#include "return.h"

/* The BLOCK macro.
*  Called as BLOCK()
*  The current task will be blocked and taken off the ready queue.
*/

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

#define BLOCK2(z)\
        _current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_block, z);\
        sched_block(sched_dispatched);\
        return RET_BLOCKED;\
        TOKENPASTE2(_block, z):\

/// @brief block the currently running task
#define BLOCK() BLOCK2(__COUNTER__)


#endif
