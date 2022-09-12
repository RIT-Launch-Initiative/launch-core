/*******************************************************************************
*
*  Name: macros.h
*
*  Purpose: Contains preprocessor macros for using the scheduler.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
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

#define CALL2(F, RET, z)\
    ({_current = TOKENPASTE2(&&_call, z); TOKENPASTE2(_call, z):; RetType RET = F; if(RET == RET_SLEEP || RET == RET_BLOCKED || RET == RET_YIELD){return RET;}; RET;})\

/// @brief call a function 'F' and handle the return
///        useful for calling in a task so you don't need to check for SLEEP, BLOCKED, or YIELD
///        as the task should return if either of those errors are returned
///        the task will return re-execute the line when it is scheduled again
///        if SLEEP or BLOCKED or YIELD are returned
#define CALL(F) CALL2(F, TOKENPASTE2(__ret, __COUNTER__), __COUNTER__)

/// @brief reset a task to the top
#define RESET() _current = &&_start;

// #define SAVE2(RET, z) _current = TOKENPASTE2(&&_save, z); return RET; TOKENPASTE2(_save, z):
//
// /// @brief save location in a task
// #define SAVE(RET) SAVE2(RET, __COUNTER__)

#endif
