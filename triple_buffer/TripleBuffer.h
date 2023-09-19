/******************************************************************************
*  Name: TripleBuffer.h
*
*  Purpose: triple buffer implementation
*
*  Author: Will Merges
*
******************************************************************************/

#ifndef TRIPLE_BUFFER_H
#define TRIPLE_BUFFER_H

#include <stdlib.h>
#include <stdint.h>

// there are three buffers
// one is "dirty" and is used by the writer, but not be read as it could be incomplete
// one is "clean" and is unused by the writer and reader
// one is "snap" and is a previous complete write that can be read by the reader (a "snapshot")
// through swapping these buffers we can achieve concurrent reads and writes at different rates

// the 8-bit control field is layed out as follows (big endian notation)
// | unused | new write | dirty 1 | dirty 1 | clean 1 | clean 0 | snap 1 | snap 0 |
//
// new write is a flag that says if a new write has occurred
// dirty 1 and dirty 0 are the index of the current dirty buffer
// clean 1 and clean 0 are the index of the current clean buffer
// snap 1 and snap 0 are te index of the current snap buffer

// implementation heavily influenced from https://github.com/remis-thoughts/blog/blob/master/triple-buffering/src/main/md/triple-buffering.md

#define NEW_WRITE(ctl)   (ctl & 0b01000000)
#define DIRTY_INDEX(ctl) ((ctl & 0b00110000) > 4)
#define CLEAN_INDEX(ctl) ((ctl & 0b00001100) > 2)
#define SNAP_INDEX(ctl)  (ctl & 0b00000011)

/// @brief a triple buffer
/// @tparam TYPE    the type of each buffer
/// Use 'write' to allocate a buffer for writing to
/// the next call to 'write' will allocate a new buffer and flush the old one to be read
/// Use 'read' to allocate a buffer for reading (or return the same buffer if no writes were made)
/// This is completely thread/ISR safe
/// If the rate of writes is greater than reads, some data will be dropped
/// If the rate of reads is greater than writes, read will return the same buffer multiple times
template <typename TYPE>
class TripleBuffer {
public:
    /// @brief constructor
    TripleBuffer() {
        // initial setup is
        // new_write = 0
        // dirty = 0
        // clean = 1
        // snap = 2

        m_ctl = 0b0000110;
    }

    /// @brief obtain a buffer for reading
    /// @return a pointer to a buffer to be read from, or NULL if there
    ///         are no new writes since the last time 'read' was called
    TYPE* read() {
        uint_fast8_t ctl_curr;
        uint_fast8_t ctl_new;

        do {
            ctl_curr = m_ctl;

            if(!(ctl_curr & 0b1000000)) {
                // no new write to the dirty buffer, no reason to swap it out
                return NULL;
            }

            // swaps the clean and snap buffers
            //       | dont change dirty    | snap to clean            |  clean to snap |
            ctl_new = (ctl_curr & 0b110000) | ((ctl_curr & 0b11) << 2) | ((ctl_curr & 0b1100) >> 2);
        } while(!__sync_bool_compare_and_swap(&ctl, ctl_curr, ctl_new));

        // return the new (or current if no new write) snap buffer
        return &m_buffs[SNAP_INDEX(m_ctl)];
    }

    /// @brief obtain a buffer for writing, flushing the last write buffer
    /// @return a pointer to a buffer to be written to
    TYPE* write() {
        uint_fast8_t ctl_curr;
        uint_fast8_t ctl_new;

        do {
            ctl_curr = ctl;
            // swaps the clean and dirty buffers, also setting the new write flag
            //       | new write |  clean to dirty           |  dirty to clean             | leave snap alone
            ctl_new = 0b1000000 | ((ctl_curr & 0b1100) << 2) | ((ctl_curr & 0b110000) >> 2) | (ctl_curr & 0b11);

            // try and atomically swap ctl and ctl_new, unless ctl has changed already
        } while(!__sync_bool_compare_and_swap(&ctl, ctl_curr, ctl_new));

        // return the new dirty buffer to be written to
        return &m_buffs[DIRTY_INDEX(m_ctl)];
    }
private:
    volatile uint_fast8_t m_ctl;
    TYPE m_buffs[3];
};

#endif
