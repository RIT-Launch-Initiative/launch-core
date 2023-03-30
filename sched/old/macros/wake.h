#ifndef SCHED_MACROS_WAKE_H
#define SCHED_MACROS_WAKE_H

#include "sched/sched.h"
#include "return.h"

/* The WAKE macro.
*  Called as WAKE()
*  The current task will be blocked and taken off the ready queue.
*/

/// @brief wake up a task with task ID 'TID'
#define WAKE(TID) sched_wake(TID)

#endif
