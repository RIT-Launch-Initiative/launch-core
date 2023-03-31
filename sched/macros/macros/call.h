#ifndef SCHED_MACROS_CALL_H
#define SCHED_MACROS_CALL_H

#include "sched/sched.h"
#include "return.h"
#include "sched/macros/jump_table.h"

/* The CALL macro.
*  Called as RetType ret = CALL(E)
*  The expression E will be evaluated. E must return a RetType. If the expression
*  returns blocked, yield, or sleep, the execution will return to the scheduler.
*  Used for calling functions that require returning execution to the scheduler
*  depending on the return type.
*
*  The return value of E is returned to 'ret' if it was not blocked, sleep, or
*  yield.
*/

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)


#define CALL2(F, RET, z)\
    ({sched_jump[sched_dispatched].jumps[sched_jump[sched_dispatched].size++] = TOKENPASTE2(&&_call, z); TOKENPASTE2(_call, z):; RetType RET = F; if(RET == RET_YIELD){return RET;}; RET;})\

/// @brief call a function 'F' and handle the return
///        useful for calling in a task so you don't need to check for SLEEP, BLOCKED, or YIELD
///        as the task should return if either of those errors are returned
///        the task will return re-execute the line when it is scheduled again
///        if SLEEP or BLOCKED or YIELD are returned
#define CALL(F) CALL2(F, TOKENPASTE2(__ret, __COUNTER__), __COUNTER__)

#endif
