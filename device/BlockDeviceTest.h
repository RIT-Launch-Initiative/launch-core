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
     * Construct a silent block device tester (RetType is the only status
	 * indicator)
     * @param dev 		Device under test
     */
    BlockDeviceTest(BlockDevice& dev):
            m_dut(dev), m_printf(&__blackhole_printf) {};

    /**
     * Construct a block device tester
     * @param dev 		Device under test
     * @param printer 	printf-style function to give debug output
     */
    BlockDeviceTest(BlockDevice& dev, int (*printer) (const char*, ...)):
            m_dut(dev), m_printf(printer) {};


    /**
     * Configure how many characters (hex) the address should be printed to
     * @param addr_width
     */
    void config(int addr_width) {
        dut_addr_width = addr_width;
    }

    /**
     * Write from buff to the device at block, read it back, compare
     * @param block the block to write to
     * @param str the string to write and then compare to
     * @return RET_SUCCESS if match, RET_ERROR otherwise
     */
    RetType write_read_compare(size_t block, char* str) {
        RESUME();
        size_t len;
        len = strlen(str) + 1;

        if (len > BLOCK_SIZE) {
            m_printf("String with null is too long (%d) for one block (%d)\n", len, BLOCK_SIZE);
            return RET_ERROR;
        }
        RetType ret;

        clear_buff();
        memcpy(m_buff, (uint8_t*) str, len);

        m_printf("Writing to device at block 0x%0*X... ", dut_addr_width, block);

        m_dut.obtain();
        ret = CALL(m_dut.write(block, m_buff));
        if (RET_ERROR == ret) {
            m_printf("failed.\n");
        } else {
            m_printf("succeeded.\n\tSENT:\t%.*s\n", m_dut.get_block_size(), m_buff);
        }
        ret = CALL(read_compare(block, str));
        m_dut.release();

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
        m_printf("Reading from device at block 0x%0*X... ", dut_addr_width, block);

        clear_buff();
        ret = CALL(m_dut.read(block, m_buff));

        if (RET_ERROR == ret) {
            m_printf("failed.\n");
        } else {
            m_printf("succeeded.\n\tREAD:\t%.*s\n", m_dut.get_block_size(), m_buff);

			if (0 == strncmp(str, (char*) m_buff, m_dut.get_block_size())) {
				m_printf("The buffers are equal.\n");
				ret = RET_SUCCESS;
			} else {
				m_printf("The buffers are NOT equal.\n");
				ret = RET_ERROR;
			}
        }

        RESET();
        return ret;
    }
    /**
     * Read a block from the device and print it out
     * @param block the block to read
     * @param str the string to compare to
     * @return RET_SUCCESS if match, RET_ERROR otherwise
     */
    RetType print(size_t block) {
        RESUME();
        RetType ret;
        m_printf("Reading from device at block 0x%0*X... ", dut_addr_width, block);

        clear_buff();
        ret = CALL(m_dut.read(block, m_buff));

        if (RET_ERROR == ret) {
            m_printf("failed.\n");
        } else {
            m_printf("succeeded.\n\tREAD:\t%.*s\n", m_dut.get_block_size(), m_buff);
        }

        RESET();
        return ret;
    }
    /**
     * Print the data in a region of the device
     * @param start start block
     * @param len   how many blocks
     * @return RET_SUCCESS always
     */
    RetType print_region(size_t start, size_t len) {
        RESUME();
        static RetType ret;
        static size_t i;

        m_printf("Reading %ld blocks at 0x%0*X\n", len, dut_addr_width, start);

        for (i = 0; i < len; i++) {
            clear_buff();
            m_printf("0x%0*X: ", dut_addr_width, start + i);
            ret = CALL(m_dut.read(start + i, m_buff));
            if (RET_ERROR == ret) {
                m_printf("FAILED\n");
                continue;
            }

            if (is_empty()) {
                m_printf("EMPTY\n");
            } else {
                m_printf("\t%.*s\n", m_dut.get_block_size(), m_buff);
            }
            SLEEP(10); // give the print a chance to go through (hopefully)
        }

        RESET();
        return RET_SUCCESS;
    }

private:
    BlockDevice& m_dut; // (d)evice (u)nder (t)est
    uint8_t m_buff[BLOCK_SIZE];
    int (*m_printf) (const char*, ...);
    int dut_addr_width = 8;

    void clear_buff(void) {
        memset(m_buff, 0x00, sizeof(m_buff));
    }

    bool is_empty() {
        for (size_t i = 0; i < sizeof(m_buff); i++) {
            // either all-zeros or all-ones counts as "empty"
            if ((0x00U != m_buff[i]) && (0xFFU != m_buff[i])) {
                return false;
            }
        }
        return true;
    }

	static int __blackhole_printf(const char* fmt, ...) {
		return 1;
	}
};

#endif
