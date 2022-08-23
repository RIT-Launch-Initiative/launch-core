#ifndef QUEUE_SORT_H
#define QUEUE_SORT_H

/// @brief sorting function for use with sorted queues
///        returns true if fst should be popped before snd (i.e. snd is later in the queue)
template <typename T>
using sort_t = bool (*)(T& fst, T& snd);

#endif
