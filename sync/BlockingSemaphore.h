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
        if(m_count > 0) {
            m_count--;
        }

        // wake someone up from the queue if we can
        // TODO

        m_lock.release();


        return RET_SUCCESS;
    }

    /// @brief acquire
    /// @return
    RetType acquire() {
        // TODO check m_count
        // if it's zero, add ourselves to the queue
    }

private:
    Semaphore m_lock;
    int m_count;
    alloc::Queue<tid_t, MAX_NUM_TASKS> m_queue;
};

#endif
