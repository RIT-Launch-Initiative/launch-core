/*******************************************************************************
*
*  Name: macros.cpp
*
*  Purpose: Declares data needed for macro functionality.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/

#include "sched/macros/macros.h"

jump_table_t sched_jump[MAX_NUM_TASKS] = {0};
tid_t sched_dispatched;
