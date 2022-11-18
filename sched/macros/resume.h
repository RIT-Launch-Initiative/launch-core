#ifndef SCHED_MACROS_RESUME_H
#define SCHED_MACROS_RESUME_H

#include "sched/sched.h"
#include "return.h"

/* The RESUME macro.
*  Called as RESUME()
*  Sets up a function to be able to returned to after blocking, sleeping, or
*  yielding. Any function that uses any of the scheduler macros should have a
*  call to RESUME at the top.
*/

/// @brief resumes a task from where it last slept or blocked from
///        should be at the top of the task function
/// NOTE: this uses the fact that we can store the address of a label as a value
///       that's not a C/C++ feature but is part of GCC, so we are dependent on
///       using GCC/G++ for these macros to work
#define RESUME()\
            static bool _init = false;\
            static void* _current[static_cast<int>(MAX_NUM_TASKS)];\
            if(!_init) {\
                for(int i = 0; i < static_cast<int>(MAX_NUM_TASKS); i++) {\
                    _current[i] = &&_start;\
                }\
                _init = true;\
            }\
            goto *(_current[static_cast<int>(sched_dispatched)]);\
            _start:\

// TODO checking init feels a bit slow, not sure how else to initialize _current though

#endif
