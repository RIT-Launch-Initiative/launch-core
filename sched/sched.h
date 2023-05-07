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

/// @brief task id, any tid < 0 or > MAX_NUM_TASKS is an error
///        a tid equal to MAX_NUM_TASKS represents no task executing
typedef int tid_t;

/// @brief TID of currently dispatched task
extern tid_t sched_dispatched;

/// @brief function type to get system time
///        units are arbitrary, same units as sleep
typedef uint32_t (*time_func_t)();

/// @brief task function type
typedef RetType (*task_func_t)(void* arg);

// constants
// static const size_t SAVE_BLOCK_SIZE = 256;
static const tid_t MAX_NUM_TASKS = 64;

/// @brief save stack
///        used for storing variables from a task
// typedef struct {
//     uint8_t block[SAVE_BLOCK_SIZE];     // block to store data in
//     uint8_t* curr;                      // current position in block
// } stack_t;

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
    // stack_t stack;
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

/// @brief dispatch the next task
void sched_dispatch();

/// @brief sleep a task
/// @param
void sched_sleep(tid_t tid, uint32_t time);

/// @brief wake up a task
void sched_wake(tid_t tid);

/// @brief block a task
///        task will not be dispatched until 'sched_wake' is called
void sched_block(tid_t tid);

// /// @brief save a variable to a task
// template <typename T>
// void sched_save(tid_t tid, T* var);
//
// /// @brief restore a variable from a task
// template <typename T>
// T* sched_restore(tid_t tid);

#endif
