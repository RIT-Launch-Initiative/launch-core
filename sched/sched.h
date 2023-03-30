/*******************************************************************************
*
*  Name: sched.h
*
*  Purpose: Declares functions to use the cooperative scheduler.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef SCHED_H
#define SCHED_H

#include <stdlib.h>
#include <stdint.h>

#include "return.h"
#include "sched/config.h"

/// @brief task id, any tid < 0 or > MAX_NUM_TASKS is an error
///        a tid equal to MAX_NUM_TASKS represents no task executing
typedef int tid_t;

/// @brief function type to get system time
///        units are arbitrary, same units as sleep
typedef uint32_t (*time_func_t)();

/// @brief task function type
typedef RetType (*task_func_t)(void* arg);

/// @brief task states
typedef enum {
    STATE_UNALLOCATED = 0,
    STATE_ACTIVE,
    STATE_SLEEPING,
    STATE_BLOCKED
} state_t;

/// @brief task information
typedef struct task_s {
    state_t state;
    task_func_t func;
    void* arg;
    tid_t tid;
    uint32_t wake_time;
    struct task_s** ready_loc; // address of the task pointer on the ready queue
    struct task_s** sleep_loc; // address of the task pointer on the ready queue
} task_t;

/// @brief initialize the scheduler
/// @return 'true' on success, 'false' on failure
bool sched_init(time_func_t func);

/// @brief get the system time used by the scheduler
/// @return the system time, in units of the function passed to 'sched_init'
uint32_t sched_time();

/// @brief start a task on the scheduler
/// @param func     the task function
/// @param arg      the argument to pass the task everytime it's executed
/// @return the started task task id, or -1 on error
tid_t sched_start(task_func_t func, void* arg);

/// @brief select the next task
/// @return the next task scheduled, or -1 if no task is ready to be scheduled
tid_t sched_select();

/// @brief kill a task, removing it from the scheduler
/// @param tid  the tid of the task to kill
void sched_kill(tid_t tid);

/// @brief sleep a task
/// @param tid   the tid of the task to sleep
/// @param time  the number of scheduler ticks to sleep the task for
void sched_sleep(tid_t tid, uint32_t time);

/// @brief wake up a task
/// @param tid  the tid of the task to wake
void sched_wake(tid_t tid);

/// @brief block a task
///        task will not be dispatched until 'sched_wake' is called
/// @param tid  the tid of the task to block
void sched_block(tid_t tid);

#endif
