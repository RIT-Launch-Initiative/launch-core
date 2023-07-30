/*******************************************************************************
*
*  Name: sched.cpp
*
*  Purpose: Implements a cooperative scheduler.
*
*  Author: Will Merges
*  Author: Aaron Chan
*
*  RIT Launch Initiative
*
*******************************************************************************/
#include "sched/sched.h"
#include "queue/allocated_queue.h"

// preallocated task structures
static task_t tasks[MAX_NUM_TASKS];

// ready queue
static alloc::Queue<task_t *, MAX_NUM_TASKS> ready_q{};

static int num_tasks = 0;

// sorting function for the sleep queue
// sorts task_t's
// sort by nearest wakeup time first
bool sleep_sort(task_t *&fst, task_t *&snd) {
    if (snd->wake_time > fst->wake_time) {
        // first task should be dequeued before the second task
        return true;
    } else {
        return false;
    }
}

// sleep queue
static alloc::SortedQueue<task_t *, MAX_NUM_TASKS> sleep_q{&sleep_sort};

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
    if (!func) {
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
tid_t sched_start(task_func_t func, void *arg) {
    // find an unused task structure
    for (tid_t i = 0; i < MAX_NUM_TASKS; i++) {
        if (STATE_UNALLOCATED == tasks[i].state) {
            // we found one!
            tasks[i].state = STATE_ACTIVE;              // set active
            tasks[i].func = func;
            tasks[i].arg = arg;
            tasks[i].tid = i;
            tasks[i].sleep_loc = nullptr;

            // put the allocated task on the ready queue
            // NOTE: we assume we never fail to place a task on the ready queue.
            //       this is valid because the queue is the same size as the
            //       number of tasks allocated.
            tasks[i].ready_loc = ready_q.push(&(tasks[i]));
            num_tasks++;

            return i;
        }
    }

    // we have no free TID's, too many tasks running
    return -1;
}

// helper function to wake up tasks in the sleep queue
static void _sched_wakeup_tasks() {
    while (true) {
        task_t **task_p = sleep_q.peek();

        if (nullptr == task_p) {
            // nothing in the sleep queue
            return;
        }

        task_t *task = *task_p;

        if (get_time() >= task->wake_time) {
            // pop it off the sleep queue
            sleep_q.pop();

            // set active
            task->state = STATE_ACTIVE;
            task->sleep_loc = nullptr;

            // put it on the ready queue
            // NOTE: we assume we can always push to the ready queue
            task->ready_loc = ready_q.push(task);;
        } else {
            // this task is the earliest task we need to wake up and it isn't time yet
            return;
        }
    }
}

/// @brief select the next task
/// @return the next task scheduled, or NULl if no task is ready to be scheduled
task_t *sched_select() {
    // wakeup any sleeping tasks
    _sched_wakeup_tasks();

    task_t **task_p = ready_q.peek();

    if (nullptr == task_p) {
        // nothing ready
        return nullptr;
    }

    ready_q.pop();
    task_t *task = *task_p;

    // requeue the task
    task->ready_loc = ready_q.push(task);

    return task;
}

/// @brief kill a task, removing it from the scheduler
/// @param tid  the tid of the task to kill
void sched_kill(tid_t tid) {
    if (tid <= 0 || tid >= num_tasks) {
        return;
    }

    task_t *task = &(tasks[tid]);

    // if the task is sleeping, take it off the sleep queue
    if (nullptr != task->sleep_loc) {
        sleep_q.remove(task->sleep_loc);
        task->sleep_loc = nullptr;
        // NOTE: this can be an else because a task cannot be on more than one queue at once
    } else if (nullptr != task->ready_loc) {
        // if the task is on the ready queue, remove it
        ready_q.remove(task->ready_loc);
        task->ready_loc = nullptr;
    }

    task->state = STATE_UNALLOCATED;
    num_tasks--;
}

/// @brief sleep a task
/// @param
void sched_sleep(tid_t tid, uint32_t time) {
    if (tid <= 0 || tid >= num_tasks) {
        return;
    }

    task_t *task = &(tasks[tid]);

    // if the task is already sleeping, take it off the sleep queue
    // this guarantees when it's placed back on the queue it's sorted properly
    if (nullptr != task->sleep_loc) {
        sleep_q.remove(task->sleep_loc);
        // new sleep_loc set later
        // NOTE: this can be an else because a task cannot be on more than one queue at once
    } else if (nullptr != task->ready_loc) {
        // if the task is on the ready queue, remove it
        ready_q.remove(task->ready_loc);
        task->ready_loc = nullptr;
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
    if (tid <= 0 || tid >= num_tasks) {
        return;
    }

    task_t *task = &(tasks[tid]);

    if (STATE_BLOCKED != task->state && STATE_SLEEPING != task->state) {
        // this task is already woken and not blocked
        return;
    }

    if (nullptr != task->ready_loc) {
        // nothing to wake from, already on the ready queue
        return;
    }

    // if the task is on the sleep queue, remove it
    if (nullptr != task->sleep_loc) {
        sleep_q.remove(task->sleep_loc);
        task->sleep_loc = nullptr;
    }

    // put it on the ready queue
    // NOTE: we assume we can always push
    task->ready_loc = ready_q.push(task);
    task->state = STATE_ACTIVE;
}

/// @brief block a task
///        task will not be dispatched until 'sched_wake' is called
void sched_block(tid_t tid) {
    if (tid <= 0 || tid >= num_tasks) {
        return;
    }

    task_t *task = &(tasks[tid]);

    // remove this task from any queues it's on
    // NOTE: this is an else if because a task should only ever be on one queue
    if (nullptr != task->ready_loc) {
        ready_q.remove(task->ready_loc);
        task->ready_loc = nullptr;
    } else if (nullptr != task->sleep_loc) {
        sleep_q.remove(task->sleep_loc);
        task->sleep_loc = nullptr;
    }

    task->state = STATE_BLOCKED;
}
