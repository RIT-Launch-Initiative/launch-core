/*******************************************************************************
*
*  Name: jump_table.c
*
*  Purpose: Declares the jump table for use with scheduler macros.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/

#include "sched/macros/jump_table.h"

jump_table_t sched_jump[MAX_NUM_TASKS];
tid_t sched_dispatched;
