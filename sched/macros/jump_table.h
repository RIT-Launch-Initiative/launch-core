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

#include <stdint.h>

#include "sched/config.h"
#include "sched/sched.h"

// maxmimum call depth supported
static const size_t MAX_CALL_DEPTH = 64;

/// @brief a jump stack for a task
typedef struct {
    // the address of labels to jump to
    void* jumps[MAX_CALL_DEPTH];

    // the current size of the table
    size_t size;

    // the current index in the table
    size_t index;
} jump_table_t;

// externally linked jump tables, one per possible task
extern jump_table_t sched_jump[MAX_NUM_TASKS];

// externally linked, currently executing task
extern tid_t sched_dispatched;

#endif
