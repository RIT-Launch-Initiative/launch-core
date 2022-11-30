/*******************************************************************************
*
*  Name: LinuxBlockDevice.h
*
*  Purpose: Implements a block device in Linux for testing. Actually allocates
*           a large file to access as a fake device.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef LINUX_BLOCK_DEVICE_H
#define LINUX_BLOCK_DEVICE_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

#include "device/BlockDevice.h"
#include "return.h"
#include "sched/macros.h"
#include "queue/allocated_queue.h"


class LinuxBlockDevice : public BlockDevice {
public:
    /// @brief constructor
    LinuxBlockDevice(const char* file, size_t block_size, size_t num_blocks) :
                                                    m_file(file),
                                                    m_blockSize(block_size),
                                                    m_numBlocks(num_blocks),
                                                    m_data(NULL),
                                                    BlockDevice("Linux Block Device") {};

    /// @brief initialize the device
    RetType init() {
        // create a file to store our "blocks" in
        FILE* f = fopen(m_file, "a+");
        if(f == NULL) {
            return RET_ERROR;
        }

        // make it the right size
        if(0 != ftruncate(fileno(f), m_blockSize * m_numBlocks)) {
            return RET_ERROR;
        }

        // mmap it to an address
        m_data = mmap(NULL, m_blockSize * m_numBlocks, PROT_READ | PROT_WRITE,
                      MAP_SHARED, fileno(f), 0);

        if(m_data == (void*) -1) {
            perror("mmap fail");
            return RET_ERROR;
        }

        // we can now close the file, our mapping stays valid
        fclose(f);

        return RET_SUCCESS;
    }

    /// @brief obtain the device, always successfull
    RetType obtain() {
        return RET_SUCCESS;
    }

    /// @brief release the device, always successfull
    RetType release() {
        return RET_SUCCESS;
    }

    /// @brief poll the device
    RetType poll() {
        // sync our mapped pages with disk
        if(0 != msync(m_data, m_blockSize * m_numBlocks, MS_SYNC | MS_INVALIDATE)) {
            return RET_ERROR;
        }

        // wake up any tasks waiting for operations to complete
        tid_t* task = m_queue.peek();

        while(task != NULL) {
            m_queue.pop();
            WAKE(*task);

            task = m_queue.peek();
        }

        return RET_SUCCESS;
    }

    /// @brief write to a block
    /// @param block    the block number to write to
    /// @param data     the data to write to the block
    ///                 must be a buffer of at least BLOCK_SIZE bytes
    /// @return 'true' if the entire block was written successfully
    RetType write(size_t block, uint8_t* data) {
        RESUME();

        if(block >= m_numBlocks) {
            return RET_ERROR;
        }

        memcpy((uint8_t*)m_data + (block * m_blockSize), data, m_blockSize);

        // block after, waiting for data to be synced with disk
        RetType ret = CALL(block_task());

        RESET();
        return ret;
    }

    /// @brief read from a block
    /// @param block    the block number to read from
    /// @param buff     the buffer to read into
    ///                 must be a buffer of at least BLOCK_SIZE bytes
    /// @return 'true' if the entire block was read successfully
    RetType read(size_t block, uint8_t* buff) {
        RESUME();

        if(block >= m_numBlocks) {
            return RET_ERROR;
        }

        // block first, waiting for any updates to the device
        RetType ret = CALL(block_task());

        memcpy(buff, (uint8_t*)m_data + (block * m_blockSize), m_blockSize);

        RESET();
        return ret;
    }

    /// @brief get the block size of the device
    /// @return the block size of the device
    size_t getBlockSize() {
        return m_blockSize;
    }

    /// @brief get the number of blocks in the device
    /// @return the number of blocks
    size_t getNumBlocks() {
        return m_numBlocks;
    }

private:
    /// @brief helper function which blocks the current task
    /// @return
    RetType block_task() {
        RESUME();

        if(!m_queue.push(sched_dispatched)) {
            return RET_ERROR;
        }

        BLOCK();
        RESET();
        return RET_SUCCESS;
    }

    const char* m_file;
    size_t m_blockSize;
    size_t m_numBlocks;

    // memory mapped file
    void* m_data;

    // queues TID of blocked tasks
    alloc::Queue<tid_t, MAX_NUM_TASKS> m_queue;
};

#endif
