#ifndef SCHED_MACROS_WAKE_H
#define SCHED_MACROS_WAKE_H

#include "sched/sched.h"
#include "return.h"

/* The WAKE macro.
*  Called as WAKE(TID)
*  The task with TID will be added back to the ready queue.
*/

/// @brief wake up a task with task ID 'TID'
#define WAKE(TID) sched_wake(TID)

#endif
