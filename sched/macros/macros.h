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

// NOTE: sched/macros/jump_table.cpp must be compiled to use macros!


// ** macros for use in scheduler tasks ** //

// RESUME()
//   Should be placed at the very beginning of a function. Sets up the function
//   to be able to get blocked and return to execution at the line it was
//   blocked at. Variables are not saved between executions on the scheduler, so
//   use statics if storing information before blocking. Be aware this static
//   could get modified by another task or if one task calls the same function
//   twice.

// RESET()
//   Deprecated, unnecessary to use but if included acts as a no-op.

// SLEEP(T)
//   Sleep the current running task for 'T' ticks (in system clock time).
//   Returns RET_YIELD, all functions higher in the call stack should see this
//   return result and get execution back to the scheduler ASAP.

// BLOCK()
//   Block the current running task indefinitely, only unblocked when WAKE is
//   called on the blocked task's task ID. Returns RET_YIELD, all functions
//   lower in the call stack should see this return result and get execution
//   back to the scheduler ASAP. When the task blocked is scheduled again,
//   execution begins right after the macro.

// WAKE(TID)
//   Wake the task with task ID 'TID', Puts that task back on the ready queue if
//   it is currently blocked or sleeping.

// YIELD()
//   Yield the currently executing task's time back to the scheduler. Task stays
//   in the ready queue but is done executing for now. Return RET_YIELD, all
//   functions lower in the call stack should see this return result and get
//   execution back to the scheduler ASAP. When task is scheduled again,
//   execution begins right after the macro.

// CALL(EXP)
//   Execute the expression 'EXP' and handle the return. 'EXP' must return a
//   RetType. Used to call functions that may sleep, block, or yield. If
//   RET_YIELD is returned, RET_YIELD is returned immediately in order to get
//   execution back to the scheduler, the next time the task is scheduled
//   execution will begin before evaluating 'EXP' so it is evaluated again. If
//   RET_SUCCESS or RET_ERROR are returned, the macro evaluates to that result.


// see test/example.cpp for an example of how these macros should be used

#include <stdint.h>

#include "sched/config.h"
#include "sched/sched.h"

// maxmimum call depth supported
// if call depth is exceeded, all macros that may block will return RET_ERROR
static const size_t MAX_CALL_DEPTH = 64;

/// @brief a jump table for a task
typedef struct {
    // the address of labels to jump to
    void* jumps[MAX_CALL_DEPTH];

    // the current size of the table
    size_t size;

    // the current index in the table
    size_t index;
} jump_table_t;

// externally linked jump tables, one per task
extern jump_table_t sched_jump[MAX_NUM_TASKS];

// externally linked, currently executing task ID
extern tid_t sched_dispatched;


/// @brief dispatches a single task in the macro-based scheduler.
///        assumes scheduler is initialized.
///        this function should be called repeatedly to dispatch new tasks.
///
/// NOTE:  any task that returns RET_ERROR will be killed.
inline void dispatch() {
    task_t* task = sched_select();

    if(NULL != dispatch) {
        sched_dispatched = task->tid;
        sched_jump[sched_dispatched].index = 0;

        if(RET_ERROR == task->func(task->arg)) {
            sched_kill(sched_dispatched);
        }
    }
}

// includes for the macros //

#include "sched/macros/macros/resume.h"
#include "sched/macros/macros/reset.h"
#include "sched/macros/macros/sleep.h"
#include "sched/macros/macros/block.h"
#include "sched/macros/macros/wake.h"
#include "sched/macros/macros/yield.h"
#include "sched/macros/macros/call.h"

#endif