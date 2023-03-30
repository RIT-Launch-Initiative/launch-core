/*******************************************************************************
*
*  Name: jump_table.h
*
*  Purpose: Defines the jump table for use with scheduler macros.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef JUMP_TABLE_H
#define JUMP_TABLE_H

#include "sched/config.h"

/// @brief a jump stack for a task
typedef struct {
    // the address of labels to jump to
    void* jumps[MAX_CALL_DEPTH];

    // the current size of the stack
    size_t size;
} jump_stack_t;

extern jump_stack_t sched_jump_table[MAX_NUM_TASKS];

/// @brief pop the next jump from a tasks jump table
inline void* jt_pop(tid_t tid) {
    sched_jump_table[tid].size--;
}

/// @brief push a label onto the jump table
/// NOTE: no protection here if you blow through the stack!
///       this is intended to execute fast as it may be called for every
///       single function invocation in a call stack
inline void* jt_push(tid_t tid) {
    return &(sched_jump_table[tid].jumps[sched_jump_table[tid].size++]);
}

#endif
