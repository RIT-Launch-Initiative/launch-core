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

// RESUME()
//   Should be placed at the very beginning of a function. Sets up the function
//   to be able to get blocked and return to execution at the line it was
//   blocked at. The execution location is stored per-task, not per-function
//   so multiple tasks can call the same function and execute in different
//   places. Variables are not saved between executions on the scheduler, so
//   use statics if storing information before blocking. Be aware another task
//   could modify this static.
//   NOTE: a function using resume can NOT be recursive!!!

// RESET()
//   Should be placed at the end of a function, before any return statements but
//   after any blocking calls, such as CALL, BLOCK, YIELD, SLEEP.

// SLEEP(T)
//   Sleep the current running task for 'T' ticks (in system clock time).
//   Returns RET_SLEEP, all functions higher in the call stack should see this
//   return result and get execution back to the scheduler ASAP.

// BLOCK()
//   Block the current running task indefinitely, only unblocked when WAKE is
//   called on the blocked task's task ID. Returns RET_BLOCK, all functions
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
//   RET_BLOCKED, RET_SLEEP, or RET_YIELD are returned, the same return code is
//   returned immediately in order to get execution back to the scheduler. If
//   any of those return codes are returned, the next time the task is scheduled
//   execution will begin before evaluating 'EXP' so it is evaluated again. If
//   RET_SUCCESS or RET_ERROR are returned, the macros evaluates to that result.


// see test/example.cpp for an example of how these macros should be used


// includes for the macros //

#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "sched/macros/sleep.h"
#include "sched/macros/block.h"
#include "sched/macros/wake.h"
#include "sched/macros/yield.h"
#include "sched/macros/call.h"

#endif
