/*******************************************************************************
*
*  Name: queue_sort.h
*
*  Purpose: Defines sorting function for a sorted queue.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef QUEUE_SORT_H
#define QUEUE_SORT_H

/// @brief sorting function for use with sorted queues
///        returns true if fst should be popped before snd
template <typename T>
using sort_t = bool (*)(T& fst, T& snd);

#endif
