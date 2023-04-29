#ifndef SCHED_MACROS_RESUME_H
#define SCHED_MACROS_RESUME_H

#include "sched/sched.h"
#include "return.h"
#include "sched/macros/macros.h"

/* The RESUME macro.
*  Called as RESUME()
*  Sets up a function to be able to returned to after blocking, sleeping, or
*  yielding. Any function that uses any of the scheduler macros should have a
*  call to RESUME at the top.
*/

/// @brief resumes a task from where it last yielded to the scheduler from
///        should be at the top of the task function
/// NOTE: this uses the fact that we can store the address of a label as a value
///       that's not a C/C++ feature but is part of GCC, so we are dependent on
///       using GCC/G++ for these macros to work
#define RESUME()\
    int _cached_index = sched_jump[sched_dispatched].index; \
    if(sched_jump[sched_dispatched].index++ < sched_jump[sched_dispatched].size) { \
        goto *(sched_jump[sched_dispatched].jumps[_cached_index]); \
    } \

#endif
