/*******************************************************************************
*
*  Name: Semaphore.h
*
*  Purpose: Provide implementation for a semaphore
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "return.h"
#include "sched/macros.h"

/// NOTE: this semaphore uses spin locks! It is the most primitive of semaphores
///       it can be used to implement other semaphores such as a "BlockingSemaphore" that uses scheduler blocking
class Semaphore {
public:
    /// @brief constructor
    /// @param val  initial value
    Semaphore(int val) : m_val(val) {};

    /// @brief increment the semaphore, releasing a resource
    /// @return
    void release() {
        m_val++;
    }

    /// @brief decrement the semaphore, acquiring a resource
    /// @return
    /// TODO use __atomic_compare_exchange_n GCC builtin
    void acquire() {
        int expected = m_val;

        while(1) {
            if(expected) {
                if(__atomic_compare_exchange_n(&m_val, &expected, expected - 1, false, memorder, memorder)) {
                    break;
                } // otherwise m_val changed since we cached it in 'expected', try again
            } else {
                // expected is 0, no resources available, try again
                expected = m_val;
            }
        }
    }

private:
    int m_val;
};

#endif
