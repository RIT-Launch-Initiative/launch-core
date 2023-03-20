/*******************************************************************************
*
*  Name: sched.cpp
*
*  Purpose: Implements a cooperative scheduler.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#include "sched/sched.h"
#include "queue/allocated_queue.h"

// global TID for currently dispatched thread
// a TID equal to the max num tasks represents "system" execution
tid_t sched_dispatched = MAX_NUM_TASKS;

// preallocated task structures
static task_t tasks[MAX_NUM_TASKS];

// ready queue
static alloc::Queue<task_t*, MAX_NUM_TASKS> ready_q{};

// sorting function for the sleep queue
// sorts task_t's
// sort by nearest wakeup time first
bool sleep_sort(task_t*& fst, task_t*& snd) {
    if(snd->wake_time > fst->wake_time) {
        // first task should be dequeued before the second task
        return true;
    } else {
        return false;
    }
}

// sleep queue
static alloc::SortedQueue<task_t*, MAX_NUM_TASKS> sleep_q{&sleep_sort};

// dummy time function so we don't segfault if someone forgets to call 'sched_init'
// always returns 0
uint32_t dummy_time() {
    return 0;
}

// function to call to get system time
static time_func_t get_time = &dummy_time;

/// @brief initialize the scheduler
/// @return 'true' on success, 'false' on failure
bool sched_init(time_func_t func) {
    if(!func) {
        return false;
    }

    get_time = func;
    return true;
}

/// @brief get the system time used by the scheduler
/// @return the system time, in units of the function passed to 'sched_init'
inline uint32_t sched_time() {
    return get_time();
}

/// @brief start a task on the scheduler
/// @param func     the function to start runnning at
/// @param arg      the argument to pass the task everytime it's executed
/// @return the started task task id, or -1 on error
tid_t sched_start(task_func_t func, void* arg) {
    // find an unused task structure
    for(tid_t i = 0; i < MAX_NUM_TASKS; i++) {
        if(STATE_UNALLOCATED == tasks[i].state) {
            // we found one!
            tasks[i].state = STATE_ACTIVE;              // set active
            // tasks[i].stack.curr = tasks[i].stack.block; // reset stack
            tasks[i].func = func;
            tasks[i].arg = arg;
            tasks[i].tid = i;
            tasks[i].sleep_loc = NULL;

            // put the allocated task on the ready queue
            // NOTE: we assume we never fail to place a task on the ready queue.
            //       this is valid because the queue is the same size as the
            //       number of tasks allocated.
            tasks[i].ready_loc = ready_q.push(&(tasks[i]));

            return i;
        }
    }

    // we have no free TID's, too many tasks running
    return -1;
}

// helper function to wake up tasks in the sleep queue
void _sched_wakeup_tasks() {
    while(1) {
        task_t** task_p = sleep_q.peek();

        if(NULL == task_p) {
            // nothing in the sleep queue
            return;
        }

        task_t* task = *task_p;

        if(get_time() >= task->wake_time) {
            // pop it off the sleep queue
            sleep_q.pop();

            // set active
            task->state = STATE_ACTIVE;
            task->sleep_loc = NULL;

            // put it on the ready queue
            // NOTE: we assume we can always push to the ready queue
            task->ready_loc = ready_q.push(task);;
        } else {
            // this task is the earliest task we need to wake up and it isn't time yet
            return;
        }
    }
}

/// @brief dispatch the next task
void  sched_dispatch() {
    while(1) {
        // wakeup any sleeping tasks
        _sched_wakeup_tasks();

        task_t** task_p = ready_q.peek();

        if(NULL == task_p) {
            // nothing to dispatch
            break;
        }

        ready_q.pop();
        task_t* task = *task_p;
        task->ready_loc = NULL;

        // dispatch the task
        sched_dispatched = task->tid;
        if(RET_ERROR == task->func(task->arg)) {
            // don't put back on the ready queue
            // free this task
            task->state = STATE_UNALLOCATED;
            break;
        }

        if(STATE_ACTIVE == task->state) {
            // if the task was slept or blocked, don't put it back on the queue

            task->ready_loc = ready_q.push(task);
            // NOTE: we again assume we can always push to the ready queue
        }

        break;
    }

    sched_dispatched = MAX_NUM_TASKS;
}

/// @brief sleep a task
/// @param
void sched_sleep(tid_t tid, uint32_t time) {
    task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked

    // if the task is already sleeping, take it off the sleep queue
    // this guarantees when it's placed back on the queue it's sorted properly
    if(NULL != task->sleep_loc) {
        sleep_q.remove(task->sleep_loc);
        // new sleep_loc set later
    // NOTE: this can be an else because a task cannot be on more than one queue at once
    } else if(NULL != task->ready_loc) {
    // if the task is on the ready queue, remove it
        ready_q.remove(task->ready_loc);
        task->ready_loc = NULL;
    }

    // set the state and wake time
    task->state = STATE_SLEEPING;
    task->wake_time = get_time() + time;

    // place the task on the sleep queue
    // NOTE: we assume we can always push
    task->sleep_loc = sleep_q.push(task);
}

/// @brief wake up a task
void sched_wake(tid_t tid) {
    task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked

    if(NULL != task->ready_loc) {
        // nothing to wake from, already on the ready queue
        return;
    }

    // if the task is on the sleep queue, remove it
    if(NULL != task->sleep_loc) {
        sleep_q.remove(task->sleep_loc);
        task->sleep_loc = NULL;
    }

    // put it on the ready queue
    // NOTE: we assume we can always push
    task->ready_loc = ready_q.push(task);
    task->state = STATE_ACTIVE;
}

/// @brief block a task
///        task will not be dispatched until 'sched_wake' is called
void sched_block(tid_t tid) {
    task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked

    // remove this task from any queues it's on
    // NOTE: this is an else if because a task should only ever be on one queue
    if(NULL != task->ready_loc) {
        ready_q.remove(task->ready_loc);
        task->ready_loc = NULL;
    } else if(NULL != task->sleep_loc) {
        sleep_q.remove(task->sleep_loc);
        task->sleep_loc = NULL;
    }

    task->state = STATE_BLOCKED;
}

// /// @brief save a variable to a task
// template <typename T>
// void sched_save(tid_t tid, T* var) {
//     task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked
//     stack_t* stack = &(task->stack);
//
//     // TODO no error checking for saving
//     // if you save too much, it will start overwriting memory
//
//     uint8_t* data = (uint8_t*)(var);
//     // TODO this is basically memcpy, but don't want to be dependent on libc yet
//     for(size_t i = 0; i < sizeof(T); i++) {
//         *(stack->curr) = data[i];
//         stack->curr++;
//     }
// }
//
// /// @brief restore a variable from a task
// template <typename T>
// T* sched_restore(tid_t tid) {
//     task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked
//     stack_t* stack = &(task->stack);
//
//     stack->curr -= sizeof(T);
//     return (T*)(stack->curr + sizeof(T));
// }
