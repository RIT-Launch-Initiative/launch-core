/*******************************************************************************
*
*  Name: ChainFS.h
*
*  Purpose: Chain File System
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef CHAIN_FS_H
#define CHAIN_FS_H

#include <string.h>

#include "filesystem/FileSystem.h"
#include "device/BlockDevice.h"
#include "filesystem/ChainFS/descriptors.h"
#include "sched/macros.h"
#include "pool/pool.h"


namespace chainfs {

// maximum block size supported
static const size_t MAX_BLOCK_SIZE = 2048;

// maximum number of files open at once supported
static const size_t MAX_OPEN_FILES = 16;


class ChainFS : public FileSystem {
public:
    /// @brief constructor
    ChainFS(BlockDevice& dev) : m_dev(dev),
                                last_block(0xFFFFFFFF),
                                free_block(0xFFFFFFFF) {
        m_blockSize = dev.getBlockSize();
        m_numBlocks = dev.getNumBlocks();
    }

    /// information kept on an opened file
    typedef struct {
        bool open;                           // if this file is open or not
        uint8_t rdata[MAX_BLOCK_SIZE];       // temporary buffer for reading data blocks
        uint8_t wdata[MAX_BLOCK_SIZE];       // temporary buffer for writing data blocks
        uint8_t rdesc[MAX_BLOCK_SIZE];       // temporary buffer for descriptor block of current segment being read
        uint8_t wdesc[MAX_BLOCK_SIZE];       // temporary buffer for descriptor block of current segment being written
        uint32_t rdesc_block;                // block number buffered in 'rdesc'
        uint32_t wdesc_block;                // block number buffered in 'wdesc'
        uint32_t read_block;                 // current block being read
        uint32_t read_offset;                // offset in current 'read_block'
        uint32_t write_block;                // next block to write to
        uint32_t write_offset;               // next byte to write in 'write_block'
        uint32_t name_block;                 // block that contains name descriptor for this file
    } File_t;

    /// @brief initialize
    RetType init() {
        RESUME();

        // each descriptor needs to fit on page
        // if we can't do that, we can't support the underlying device
        if(m_blockSize < sizeof(descriptor_t)) {
            return RET_ERROR;
        }

        // push file descriptor numbers onto queue
        for(int i = 0; i < MAX_OPEN_FILES; i++) {
            if(!m_queue.push(i)) {
                return RET_ERROR;
            }

            // zero file descriptor
            File_t* desc = &(m_files[i]);
            memset(desc, 0, sizeof(File_t));

            // set as not open (just in case)
            desc->open = false;
        }

        // read through the linked list of descriptors
        // record the last descriptor on the last and the last free descriptor seen

        RetType ret;

        static descriptor_t* desc;
        static uint32_t block;
        static uint32_t prev_block;

        block = 0;
        prev_block = 0;

        while(block != 0xFFFFFFFF) {
            ret = CALL(m_dev.read(block, m_block));

            if(ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            desc = (descriptor_t*)m_block;
            prev_block = block;

            block = ((free_descriptor_t*)desc)->header.next_block;

            // store the last free block we see
            if(((free_descriptor_t*)desc)->header.type == FREE_DESCRIPTOR) {
                free_block = prev_block;
            }

        }

        last_block = prev_block;

        return RET_SUCCESS;
    }

    /// @brief open a file
    ///        Creates a new file if there isn't one already
    /// @param filename     the name of the file
    /// @param fd           set to the file descriptor of the opened file
    /// @param new_file     set to true if a new file was created, false otherwise
    /// @return file descriptor, or -1 on error
    RetType open(const char* filename, int* fd, bool* new_file = NULL) {
        RESUME();

        RetType ret = RET_SUCCESS;

        // strlen does not include NULL byte, so check if >= max string we can store
        if(strlen(filename) >= MAX_FILENAME_SIZE) {
            return RET_ERROR;
        }

        // allocate a file descriptor for this file
        static int* fd_ptr;
        fd_ptr = m_queue.peek();

        if(NULL == fd_ptr) {
            // too many open files already
            return RET_ERROR;
        }

        m_queue.pop();

        static name_descriptor_t* name_desc;
        static bool found;
        static uint32_t name_block;

        found = false;
        name_block = 0;

        // search through the entire linked list of descriptors looking for this filename
        while(name_block != 0xFFFFFFFF) {
            ret = CALL(m_dev.read(name_block, m_block));

            if(ret != RET_SUCCESS) {
                goto out;
            }

            name_desc = (name_descriptor_t*)m_block;
            if(desc->header.type == NAME_DESCRIPTOR) {
                if(0 == strncmp(filename, name_desc->name, MAX_FILENAME_SIZE)) {
                    // this is our file
                    found = true;
                    break;
                }
            }

            // check the next descriptor
            name_block = desc->header.next_block;
        }

        if(!found) {
            // make a new name descriptor for this file

            ret = CALL(get_free_blocks(1, &name_block));

            if(ret != RET_SUCCESS) {
                goto out;
            }

            name_desc = (name_descriptor_t*)m_block;
            name_desc->data = 0xFFFFFFFF; // no data blocks allocated yet // TODO allocate data block first then this
            strncpy(desc->name, filename, MAX_FILENAME_SIZE); // copy filename

            static descriptor_t new_desc;
            new_desc = *((descriptor_t*)name_desc);

            ret = CALL(add_descriptor(&new_desc, name_block, NAME_DESCRIPTOR, sizeof(name_descriptor_t)));

            if(ret != RET_SUCCESS) {
                goto out;
            }

            // at this point we've added a name descriptor for this file
            // we need to add one data descriptor

            static uint32_t data_block;
            ret = CALL(get_free_blocks(1, &data_block));

            if(ret != RET_SUCCESS) {
                goto out;
            }

            // buffer in the data descriptor we just allocated
            ret = CALL(m_dev.read(data_block, m_block));

            if(ret != RET_SUCCESS) {
                goto out;
            }

            data_descriptor_t* data_desc = (data_descriptor_t*)m_block;

            // TODO setup this data descriptor

            new_desc = *((descriptor_t*)data_desc);
            ret = CALL(add_descriptor(&new_desc, data_block, DATA_DESCRIPTOR, sizeof(data_descriptor_t)));

            if(ret != RET_SUCCESS) {
                goto out;
            }

            // TODO write out to the name descriptor that

            // mark that we created a new file
            if(new_file != NULL) {
                *new_file = true;
            }
        } // otherwise this is a pre-existing file

        // initialize parameters in file descriptor block
        static File_t* file;
        file = &(m_files[*fd_ptr]);

        file->name_block = block;
        file->read_offset = 0;
        file->write_offset = 0;

        ret = CALL(m_dev.read(block, m_block));

        if(ret != RET_SUCCESS) {
            goto out;
        }

        // desc now points to name descriptor for this file

        // look for the read and write descriptors
        file->rdesc_block = name_desc->data;
        if(file->rdesc_block != 0xFFFFFFFF) {
            // buffer in read descriptor as first data descriptor
            ret = CALL(m_dev.read(file->rdesc_block, file->rdesc));

            if(ret != RET_SUCCESS) {
                goto out;
            }

            // buffer in the first data block to read
            data_descriptor_t* data_desc = (data_descriptor_t*)file->rdesc;
            file->read_block = data_desc->start;

            ret = CALL(m_dev.read(file->read_block, file->rdata));

            if(ret != RET_SUCCESS) {
                goto out;
            }

            // woohoo, we're setup for reading

            // look for the last data descriptor in the chain, this will be the current write descriptor
            file->wdesc_block = file->rdesc_block;
            while(file->wdesc_block != 0xFFFFFFFF) {
                ret = CALL(m_dev.read(file->write_block, file->wdesc));

                if(ret != RET_SUCCESS) {
                    goto out;
                }

                data_desc = (data_descriptor_t*)file->wdesc;
                file->wdesc_block = data_desc->next_block;
            }

            // now writing is set up
        } else {
            // there's no data descriptors for this file yet
            file->read_block = 0xFFFFFFFF;
            file->wdesc_block = 0xFFFFFFFF;
            file->write_block = 0xFFFFFFFF;
        }

        // at this point our file is all setup
        // all fields are set
        // if there are no data blocks yet, all 'block' fields are set to 0xFFFFFFFF

out:
        // put this file descriptor back on the queue if we failed to open
        if(ret != RET_SUCCESS) {
            m_queue.push(*fd);
        } else {
            // mark the file as successfully opened!
            file->open = true;
        }

        // TODO unlock FS

        *fd = *fd_ptr;

        RESET();
        return ret;
    }

    /// @brief write to a file
    /// @param fd       descriptor to file to write to
    /// @param buff     data to write
    /// @param len      number of bytes to write
    /// @return
    RetType write(int fd, uint8_t* buff, size_t len) {
        // TODO

        return RET_SUCCESS;
    }

    /// @brief read from a file
    /// @param fd       descriptor to file to read from
    /// @param buff     buffer to read data into
    /// @param len      number of bytes to read
    /// @return
    RetType read(int fd, uint8_t* buff, size_t len) {
        RESUME();

        // look up the file descriptor
        File_t* file = &(m_files[fd]);

        if(!file->open) {
            return RET_ERROR;
        }

        // TODO read what we can from file->rdata
        //      update offset
        //      if we need to read more, buffer in a new block
        //      to do that we read file->rdesc to see where the next block is
        //      if we have no more blocks, error

        RESET();
        return RET_SUCCESS;
    }

    /// @brief flush a file to persistent storage
    /// @param fd   descriptor to file to flush
    RetType flush(int fd) {
        // TODO

        return RET_SUCCESS;
    }

    /// @brief format the underlying device for this filesystem
    ///        NOTE: destructive operation!!!
    /// @return
    RetType format() {
        RESUME();

        if(m_blockSize > MAX_BLOCK_SIZE) {
            return RET_ERROR;
        }

        if(m_blockSize < sizeof(descriptor_t)) {
            return RET_ERROR;
        }

        // we just write a free descriptor to the first page
        free_descriptor_t* desc = (free_descriptor_t*)m_block;
        desc->header = {FREE_DESCRIPTOR, 0xFFFFFFFF};
        desc->start = 1;
        desc->len = m_numBlocks - 1;

        RetType ret = CALL(m_dev.write(0, m_block));

        RESET();
        return ret;
    }

private:
    // reference to underlying block device
    BlockDevice& m_dev;

    // temporary buffer used for buffering blocks
    uint8_t m_block[MAX_BLOCK_SIZE];

    // size of blocks in underlying device (in bytes)
    size_t m_blockSize;

    // number of blocks available
    size_t m_numBlocks;

    // location and copy of last descriptor read in the chain
    uint32_t last_block;

    // location and copy of currently known free descriptor in the chain
    uint32_t free_block;

    // queue of file descriptor integers
    alloc::Queue<int, MAX_OPEN_FILES> m_queue;

    // maps integers to File Descriptors
    File_t m_files[MAX_OPEN_FILES];


    /// @brief helper function to locate free blocks
    RetType get_free_blocks(uint32_t num, uint32_t* start) {
        RESUME();

        // TODO lock this function so only one task can call it at a time
        // maybe just put locks in higher level funcs

        RetType ret;
        free_descriptor_t* free_desc;

        if(free_block == 0xFFFFFFFF) {
            // there are no free blocks on the system
            return RET_ERROR;
        }

        // start looking for a free block big enough
        // start at the free descriptor at 'free_block'
        static free_descriptor_t* desc;
        static uint32_t block;
        block = free_block;

        while(1) {
            ret = CALL(m_dev.read(block, m_block));

            if(ret != RET_SUCCESS) {
                return ret;
            }

            desc = (free_descriptor_t*)m_block;
            if(desc->header.type == FREE_DESCRIPTOR) {
                if(desc->len >= num) {
                    // we found one!
                    break;
                }
            }

            // keep looking
            block = desc->header.next_block;

            if(block == 0xFFFFFFFF) {
                // wrap around
                block = 0;
            }

            if(block == free_block) {
                // we're back to where we started and didn't find anything
                block = 0xFFFFFFFF;
                break;
            }
        }

        if(block == 0xFFFFFFFF) {
            // we've looked everywhere and we don't have a chunk big enough :(
            RESET();
            return RET_ERROR;
        }

        // update the free descriptor we found
        // steal 'num' blocks
        *start = desc->start;
        desc->start += num;
        desc->len -= num;

        ret = CALL(m_dev.write(free_block, m_block));

        RESET();
        return ret;
    }

    /// @brief helper function to add a descriptor to the chain
    /// NOTE: 'desc' can not point to any memory in 'm_block' as it will be ovewritten
    RetType add_descriptor(descriptor_t* desc, uint32_t block, uint32_t type, size_t size) {
        RESUME();

        RetType ret;

        ret = CALL(m_dev.read(last_block, m_block));

        if(ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        descriptor_t* last = (descriptor_t*)m_block;
        last->header_only.next_block = block;

        ret = CALL(m_dev.write(last_block, m_block));

        if(ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        desc->header_only.type = type;
        desc->header_only.next_block = 0xFFFFFFFF;
        memcpy(m_block, (void*)desc, size);

        ret = CALL(m_dev.write(block, m_block));

        if(ret != RET_SUCCESS) {
            // NOTE: the last descriptor is still messed up! the 'next_block' link points to a bad block
            RESET();
            return ret;
        }

        last_block = block;

        RESET();
        return RET_SUCCESS;
    }
};


} // namespace chainfs

#endif
