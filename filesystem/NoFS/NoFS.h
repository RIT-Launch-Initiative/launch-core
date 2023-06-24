// Created by Yevgeniy Gorbachev on 2023-06-17.

#ifndef LAUNCH_CORE_NOFS_H
#define LAUNCH_CORE_NOFS_H

#include <string.h>
#include <stdlib.h>

#include "sched/sched.h"
#include "sched/macros.h"

#include "filesystem/FileSystem.h"
#include "device/BlockDevice.h"
#include "sync/BlockingSemaphore.h"

#include "device/platforms/stm32/swdebug.h"

#define CHECK_CALL(expr) {if (RET_SUCCESS != CALL(expr)) {RESET(); return RET_ERROR;}}
static inline int min(int a, int b) {
    return (b > a) ? b : a;
}

template <const size_t BUF_SIZE>
class NoFS {
public:
    NoFS(BlockDevice& dev): m_dev(dev), m_lock(1) {};

    /// @brief Get block device parameters
    RetType init() {
        swprint("Initializing filesystem...\n");

        size_t bs = m_dev.get_block_size();
        if (0 == bs) {
            swprint("#RED#Block device not initialized");
            return RET_ERROR;
        }
        if (bs > BUF_SIZE) {
            swprint("#RED#BUF_SIZE too small");
            return RET_ERROR;
        }

        swprint("Initialized filesystem\n");
        return RET_SUCCESS;
    };

    RetType open(file_mode_t mode) {
        if (MODE_CLOSED == m_mode) {
            m_mode = mode;
            m_reset();
            return RET_SUCCESS;
        } else {
            return RET_ERROR;
        }
    };

    RetType close() {
        if (MODE_CLOSED != m_mode) {
            m_mode = MODE_CLOSED;
            m_reset();
            return RET_SUCCESS;
        } else {
            return RET_ERROR;
        }
    };

    RetType write(uint8_t* src, size_t len) {
        RetType ret;
        RESUME();

        swprint("Writing\n");
        /// Checks
        if (MODE_WRITE != m_mode) {
            swprintf("#RED#FS: File is not open for writing\n");
            RESET();
            return RET_ERROR;
        }

        /// Body
        CALL(m_lock.acquire());

        static size_t pos;
        for (pos = 0; pos < len;) {

            // src + pos: start at current position
            // len - pos: only this many left to write
            pos += buf_write(src + pos, len - pos);

            swprintf("FS: Buffered %d of %d bytes\n", pos, len);

            if (m_dev.get_block_size() == m_buf_pos) {
                swprintf("FS: Reached buffer limit, writing to block %d\n", m_block_pos);

                ret = CALL(dev_write(m_block_pos));
                if (RET_SUCCESS != ret) {
                    swprintf("#RED#FS: Failed to write buffer to device\n");

                    RESET();
                    return RET_ERROR;
                }
                m_block_pos++;
            }
        }

        CALL(m_lock.release());

        RESET();
        return RET_SUCCESS;
    };

    RetType read(uint8_t* dst, size_t len) {
        RESUME();

        /// Checks
        if (MODE_READ != m_mode) {
            swprint("#RED#FS: File is not open for reading\n");
            RESET();
            return RET_ERROR;
        }

        /// Body
        CALL(m_lock.acquire());

        static size_t pos;
        for (pos = 0; pos < len;) {
            // src + pos: start at current position
            // len - pos: only this many left to write
            pos += buf_read(dst + pos, len - pos);

            swprintf("FS: Buffered %d of %d bytes\n", pos, len);

            if (m_dev.get_block_size() == m_buf_pos) {
                swprintf("FS: End of block, reading block 0x%04x from device\n", m_block_pos);

                CHECK_CALL(dev_read(m_block_pos));
                m_block_pos++;
            }
        }
        CALL(m_lock.release());

        RESET();
        return RET_SUCCESS;
    };

    RetType format() {
        RetType ret;
        RESUME();

        swprint("Formatting device...\n");
        CALL(m_lock.acquire());
        /// Checks
        if (MODE_CLOSED != m_mode) {
            swprint("#RED#FS: Not formatting, file is open\n");
            return RET_ERROR;
        }


        ret = CALL(m_dev.clear());
        if (RET_SUCCESS != ret) {
            swprint("#RED#FS: Failed to clear device\n");
            RESET();
            return RET_ERROR;
        }

        m_buf_pos = 0;
        m_block_pos = 0;
        memset(m_buf, 0, BUF_SIZE);
        CALL(m_lock.release());

        swprint("Formatted device...\n");

        RESET();
        return ret;
    };

    RetType flush() {
        RESUME();
        CHECK_CALL(dev_write(m_block_pos++));
        RESET();
        return RET_SUCCESS;
    };

private:
    void m_reset() {
        m_buf_pos = 0;
        m_block_pos = 0;
        memset(m_buf, 0, sizeof(m_buf));
    }

    RetType dev_read(size_t block) {
        RESUME();
        if (block > m_dev.get_num_blocks()) {
            return RET_ERROR;
        }

        RetType ret;
        ret = CALL(m_dev.read(block, m_buf));
        m_buf_pos = 0;

        RESET();
        return ret;
    };

    RetType dev_write(size_t block) {
        RESUME();
        if (block > m_dev.get_num_blocks()) {
            return RET_ERROR;
        }

        RetType ret;
        ret = CALL(m_dev.write(block, m_buf));
        m_buf_pos = 0;

        RESET();
        return ret;
    };


    size_t buf_read(void* dst, size_t len) {
        len = min(len, BUF_SIZE - m_buf_pos);
        memcpy(dst, m_buf + m_buf_pos, len);
        m_buf_pos += len;
        return len;
    };

    size_t buf_write(void* src, size_t len) {
        len = min(len, BUF_SIZE - m_buf_pos);
        memcpy(m_buf + m_buf_pos, src, len);
        m_buf_pos += len;
        return len;
    };

    /// Block device relevant properties
    BlockingSemaphore m_lock;
    BlockDevice& m_dev;

    /// Internal buffer
    uint8_t m_buf[BUF_SIZE];
    size_t m_buf_pos = 0; // position in buffer
    size_t m_block_pos = 0; // position in device (blocks)
    file_mode_t m_mode = MODE_CLOSED;
    // position in file is just m_block_pos * dev_block_size + m_buf_pos
};

#undef CHECK_CALL
#endif
