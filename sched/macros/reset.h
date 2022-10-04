#ifndef SCHED_MACROS_RESET_H
#define SCHED_MACROS_RESET_H

#include "sched/sched.h"
#include "return.h"

/* The RESET macro.
*  Called as RESET()
*  Resets the function to start executing from after RESUME() macro next time
*  it is scheduled. Should be placed at the end of a function, before the last
*  return, but after any other blocking macros such as BLOCK, CALL, SLEEP, or
*  YIELD.
*/

/// @brief reset a task to the top
#define RESET() _current = &&_start;


#endif
