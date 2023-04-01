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

// ** macros for use in scheduler tasks ** //

// NOTE: sched/macros/jump_table.cpp must be compiled to use macros!

// RESUME()
//   Should be placed at the very beginning of a function. Sets up the function
//   to be able to get blocked and return to execution at the line it was
//   blocked at. Variables are not saved between executions on the scheduler, so
//   use statics if storing information before blocking. Be aware this static
//   could get modified by another task or if one task calls the same function
//   twice.

// RESET()
//   Should be placed at the end of a function, before any return statements but
//   after any blocking calls, such as CALL, BLOCK, YIELD, SLEEP.

// SLEEP(T)
//   Sleep the current running task for 'T' ticks (in system clock time).
//   Returns RET_YIELD, all functions higher in the call stack should see this
//   return result and get execution back to the scheduler ASAP.

// BLOCK()
//   Block the current running task indefinitely, only unblocked when WAKE is
//   called on the blocked task's task ID. Returns RET_YIELD, all functions
//   higher in the call stack should see this return result and get execution
//   back to the scheduler ASAP. When the task blocked is scheduled again,
//   execution begins right after the macro.

// WAKE(TID)
//   Wake the task with task ID 'TID', Puts that task back on the ready queue if
//   it is currently blocked or sleeping.

// YIELD()
//   Yield the currently executing task's time back to the scheduler. Task stays
//   in the ready queue but is done executing for now. Return RET_YIELD, all
//   functions higher in the call stack should see this return result and get
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


// includes for the macros //

#include "sched/macros/macros/resume.h"
#include "sched/macros/macros/reset.h"
#include "sched/macros/macros/sleep.h"
#include "sched/macros/macros/block.h"
#include "sched/macros/macros/wake.h"
#include "sched/macros/macros/yield.h"
#include "sched/macros/macros/call.h"

#endif
