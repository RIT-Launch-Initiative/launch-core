#include "sched/sched.h"
#include "queue/allocated_queue.h"

// global TID for currently dispatched thread
tid_t sched_dispatched;

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

// function to call to get system time
static time_func_t get_time = NULL;

/// @brief initialize the scheduler
/// @return 'true' on success, 'false' on failure
bool sched_init(time_func_t func) {
    if(!func) {
        return false;
    }

    get_time = func;
    return true;
}

/// @brief start a task on the scheduler
/// @param func     the function to start runnning at
/// @return the started task task id, or -1 on error
tid_t sched_start(task_func_t func) {
    // find an unused task structure
    for(tid_t i = 0; i < MAX_NUM_TASKS; i++) {
        if(STATE_UNALLOCATED == tasks[i].state) {
            // we found one!
            tasks[i].state = STATE_ACTIVE;              // set active
            tasks[i].stack.curr = tasks[i].stack.block; // reset stack
            tasks[i].func = func;
            tasks[i].tid = i;
            tasks[i].queued = true;

            // put it on the ready queue
            // NOTE: we assume this never fails, since the queue size
            //       is the same as the maximum number of tasks
            ready_q.push(&tasks[i]);

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

        if(get_time() > task->wake_time) {
            // pop off the sleep queue
            sleep_q.pop();

            // set active
            task->state = STATE_ACTIVE;
            task->queued = true;

            // enqueue
            // we can do this because a task is never on both the
            // ready queue and the sleep queue
            ready_q.push(task);
            // NOTE: we again assume we can always push
        } else {
            // this task is the earliest task we need to wake up and it isn't time yet
            return;
        }
    }
}

/// @brief dispatch the next task
void sched_dispatch() {
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

        // NOTE: this a 'lazy' scheduler, when task state changes the task
        //       stays on the ready queue and is popped when ready
        if(STATE_ACTIVE != task->state) {
            if(STATE_SLEEPING == task->state) {
                // this task was slept while it was on the ready queue
                task->queued = false;

                task->sleep_loc = task_p;
                sleep_q.push(task);
                // NOTE: we again assume we can always push
                continue;
            } else {
                // blocked or something else
                // don't do anything, leave it off the ready queue
                task->queued = false;
                continue;
            }
        }

        // dispatch the task
        sched_dispatched = task->tid;
        if(RET_ERROR == task->func()) {
            // don't put back on the ready queue
            // free this task
            task->state = STATE_UNALLOCATED;
            break;
        }

        // even if it was slept or blocked, put it back on the ready queue
        // we use the lazy approach and deal with it when it's popped off the ready queue
        // NOTE: this means a task that blocks and unblocks before it's popped off the ready queue
        //       gets to "cut" in line in the queue
        //       but we do guarantee it's only on the queue once or we'd have space issues
        ready_q.push(task);
        // NOTE: we again assume we can always push

        break;
    }

    sched_dispatched = -1;
}

/// @brief sleep a task
/// @param
void sched_sleep(tid_t tid, uint32_t time) {
    task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked

    if(task->state != STATE_ACTIVE) {
        // TODO is this actually bad?
        // can't sleep a non-active task
        return;
    }

    task->state = STATE_SLEEPING;
    task->wake_time = get_time() + time;
}

/// @brief wake up a task
void sched_wake(tid_t tid) {
    task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked

    if(task->state != STATE_SLEEPING && task->state != STATE_BLOCKED) {
        // nothing to wake from
        return;
    }

    // take it off the sleep queue
    sleep_q.remove(task->sleep_loc);

    task->state = STATE_ACTIVE;

    if(!task->queued) {
        ready_q.push(task);
        // NOTE: we again assume we can always push
    } // otherwise it never left the ready queue, let it stay and skip the line
}

/// @brief block a task
///        task will not be dispatched until 'sched_wake' is called
void sched_block(tid_t tid) {
    task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked

    if(task->state != STATE_ACTIVE) {
        // TODO is this actually bad?
        // can't block a non-active task
        return;
    }

    task->state = STATE_BLOCKED;
}

/// @brief save a variable to a task
template <typename T>
void sched_save(tid_t tid, T* var) {
    task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked
    stack_t* stack = &(task->stack);

    // TODO no error checking for saving
    // if you save too much, it will start overwriting memory

    uint8_t* data = (uint8_t*)(var);
    // TODO this is basically memcpy, but don't want to be dependent on libc yet
    for(size_t i = 0; i < sizeof(T); i++) {
        *(stack->curr) = data[i];
        stack->curr++;
    }
}

/// @brief restore a variable from a task
template <typename T>
T* sched_restore(tid_t tid) {
    task_t* task = &(tasks[tid]); // TODO potential memory error, tid not bounds checked
    stack_t* stack = &(task->stack);

    stack->curr -= sizeof(T);
    return (T*)(stack->curr + sizeof(T));
}
