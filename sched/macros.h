#ifndef SCHED_MACROS_H
#define SCHED_MACROS_H

#include "sched/sched.h"
#include "return.h"

// ** macros for use in scheduler tasks ** //

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

/// @brief resumes a task from where it last slept or blocked from
///        should be at the top of the task function
/// NOTE: this uses the fact that we can store the address of a label as a value
///       that's not a C/C++ feature but is part of GCC, so we are dependent on
///       using GCC/G++ for these macros to work
#define RESUME()\
            static void* _current = &&_start;\
            goto *_current;\
            _start:\

#define SLEEP2(N, z)\
            _current = TOKENPASTE2(&&_sleep, z);\
            sched_sleep(sched_dispatched, N);\
            return RET_SLEEP;\
            TOKENPASTE2(_sleep, z):\

/// @brief sleep the currently running task for 'N' ticks
#define SLEEP(N) SLEEP2(N, __COUNTER__)

#define BLOCK2(z)\
        _current = TOKENPASTE2(&&_block, z);\
        sched_block(sched_dispatched);\
        return RET_BLOCKED;\
        TOKENPASTE2(_block, z):\

/// @brief block the currently running task
#define BLOCK() BLOCK2(__COUNTER__)

/// @brief wake up a task with task ID 'TID'
#define WAKE(TID) sched_wake(TID)

#define YIELD2(z)\
        _current = TOKENPASTE2(&&_yield, z);\
        return RET_SUCCESS;\
        TOKENPASTE2(_yield, z):\

/// @brief yield back to the scheduler
#define YIELD() YIELD2(__COUNTER__)


#define CALL2(F, RET) ({RetType RET = F; if(RET == RET_SLEEP || RET == RET_BLOCKED){return RET;}; RET;})

/// @brief call a function 'F' and handle the return
///        propagates the error if not successfull
///        useful for calling in a task so you don't need to check for SLEEP or BLOCKED
///        as the task should return if either of those errors are returned
#define CALL(F) CALL2(F, TOKENPASTE2(__ret, __COUNTER__))

#endif
