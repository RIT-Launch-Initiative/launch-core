/*******************************************************************************
*
*  Name: BlockingSemaphore.h
*
*  Purpose: Provide implementation for a semaphore utilizing scheduler blocking.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef BLOCKING_SEMAPHORE_H
#define BLOCKING_SEMAPHORE_H

#include "return.h"
#include "sched/macros.h"
#include "sync/Semaphore.h"
#include "queue/allocated_queue.h"

class BlockingSemaphore {
public:
    /// @brief constructor
    BlockingSemaphore(int count) : m_count(count), m_lock(1) {};

    /// @brief release
    /// @return
    RetType release() {
        m_lock.acquire();

        m_count++;

        // unblock the next task waiting, if there is one
        tid_t* waiting = m_queue.peek();
        if(waiting) {
            // someone is waiting, let's wake them up
            m_queue.pop();
            WAKE(*waiting);
        }

        m_lock.release();

        return RET_SUCCESS;
    }

    /// @brief acquire
    /// @return
    RetType acquire() {
        RESUME();

        while(1) {
            m_lock.acquire();

            if(m_count == 0) {
                // add the calling task to the wait queue
                if(!m_queue.push(sched_dispatched)) {
                    // no room on queue, uh oh
                    return RET_ERROR;
                }

                m_lock.release();
                BLOCK(sched_dispatched);
            } else {
                // we can obtain now
                m_count--;

                m_lock.release();
                break;
            }
        }

        RESET();
        return RET_SUCCESS;
    }

private:
    // guarantees atomic access to m_count and m_queue
    Semaphore m_lock;

    // count of the semaphore
    int m_count;

    // queue of waiting tasks
    alloc::Queue<tid_t, MAX_NUM_TASKS> m_queue;
};

#endif
