//
// Created by Yevgeniy Gorbachev on 2023-07-23.
//

#include <string.h>
#include "device/BlockDevice.h"
#include "sched/macros.h"
#include "sched/sched.h"

#ifndef BLOCKDEVICETEST_H
#define BLOCKDEVICETEST_H

template <size_t BLOCK_SIZE>
class BlockDeviceTest {
public:
    /**
     * Construct a block device tester
     * @param dev Device under test
     * @param printer printf-style function to give debug output
     */
    BlockDeviceTest(BlockDevice& dev, int (*printer) (const char*, ...)):
        m_dev(dev), m_printf(printer){};

    /**
     * Write from buff to the device at block, read it back, compare
     * @param block the block to write to
     * @param str the string to write and then compare to
     * @return RET_SUCCESS if match, RET_ERROR otherwise
     */
    RetType write_read_compare(size_t block, char* str) {
        RESUME();
        size_t len;
        len = strlen(str);

        if (len > BLOCK_SIZE) {
            m_printf("String is too long (%d) for one block (%d)\n", len, BLOCK_SIZE);
            return RET_ERROR;
        }
        RetType ret;

        memcpy(m_buff, (uint8_t*) str, len);
        memset(m_buff + len, 0x00, BLOCK_SIZE - len); // zero the rest

        m_printf("Writing to device at block %d... ", block);

        ret = CALL(m_dev.write(block, m_buff));
        if (RET_ERROR == ret) {
            m_printf("failed.\n");
        } else {
            m_printf("succeeded.\nDATA:\t%.*s\n", BLOCK_SIZE, str);
        }
        ret = CALL(read_compare(block, str));

        RESET();
        return ret;
    }

    /**
     * Compare a block in the device to buff
     * @param block the block to read
     * @param str the string to compare to
     * @return RET_SUCCESS if match, RET_ERROR otherwise
     */
    RetType read_compare(size_t block, char* str) {
        RESUME();
        size_t len;
        len = strlen(str);

        if (len > BLOCK_SIZE) {
            m_printf("String is too long (%d) for one block (%d)\n", len, BLOCK_SIZE);
            return RET_ERROR;
        }

        RetType ret;
        m_printf("Reading from device at block %d... ", block);
        ret = CALL(m_dev.read(block, m_buff));
        if (RET_ERROR == ret) {
            m_printf("failed.\n");
            RESET();
            return ret;
        } else {
            m_printf("succeeded.\nDATA:\t%.*s\n", BLOCK_SIZE, str);
        }

        if (strncmp(str, (char*) m_buff, len)) {
            m_printf("The buffers are not equal\n");
            ret = RET_ERROR;
        } else {
            m_printf("The buffers are equal\n");
            ret = RET_SUCCESS;
        }
        RESET();
        return ret;
    }

private:
    BlockDevice& m_dev;
    uint8_t m_buff[BLOCK_SIZE];
    int (*m_printf) (const char*, ...);
};
#endif
