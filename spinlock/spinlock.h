#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

// must be defined somewhere else
// clock speed in Hz
extern const uint32_t CPU_CLOCK_SPEED;

/// @brief spinlock for a period of time
/// @param ms   the number of milliseconds to spinlock for
void inline spinlock(uint32_t ms) {
    uint64_t ticks = ms * CPU_CLOCK_SPEED / 1000;
    for(uint64_t i = 0; i < ticks; i++) {};
}

#endif
