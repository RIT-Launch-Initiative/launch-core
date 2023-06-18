/**
 * Platform Independent Implementation for a W25Q Flash Memory Chip
 *
 * @author Aaron Chan, Yevgeniy Gorbachev
 */

#ifndef LAUNCH_CORE_W25Q_H
#define LAUNCH_CORE_W25Q_H

#include <string.h>

#include "sched/sched.h"
#include "sched/macros.h"

#include "device/SPIDevice.h"
#include "device/GPIODevice.h"
#include "device/BlockDevice.h"

#include "device/platforms/stm32/swdebug.h"

#define W25Q128JV_JEDEC_ID 0x00EF4018
#define CHECK_CALL(expr) {if (RET_SUCCESS != CALL(expr)) {RESET(); return RET_ERROR;}}
// longest command + block read size
#define W25Q_BUF_SIZE (8 + 256)

class W25Q : public BlockDevice {
public:

    W25Q(const char* name, SPIDevice &spi, GPIODevice &cs) :
            BlockDevice(name), m_spi(spi), m_cs(cs),  m_lock(1) {}

    RetType init() {
        uint32_t dev_id;
        RESUME();

        CHECK_CALL(read_id(&dev_id));
        if (dev_id != W25Q128JV_JEDEC_ID) {
            swprintf("Expected JEDEC ID: 0x%06X\n", W25Q128JV_JEDEC_ID);
            swprintf("Incorrect JEDEC ID: 0x%06X\n", dev_id);
            RESET();
            return RET_ERROR;
        }

        m_dev_id = dev_id;
        page_size = 0xFF + 1;
        page_count = 0xFFFF + 1;

        RESET();
        return RET_SUCCESS;
    }


    RetType write(size_t block, uint8_t *buff) override {
        RESUME();

        if (block > get_num_blocks()) {
            RESET();
            return RET_ERROR;
        };

        CHECK_CALL(mem_program(PAGE_PROGRAM, block << 8, buff, get_block_size()));

        RESET();
        return RET_SUCCESS;
    }

    RetType read(size_t block, uint8_t *buff) override {
        RESUME();

        if (block > get_num_blocks()) {
            RESET();
            return RET_ERROR;
        };

        CHECK_CALL(mem_read(block << 8, buff, get_block_size()));

        RESET();
        return RET_SUCCESS;
    }

    RetType clear() override {
        RetType ret;
        uint8_t status;
        uint8_t mask = (uint8_t) S_BUSY;

        RESUME();

        CHECK_CALL(erase(CHIP_ERASE, 0));
        CHECK_CALL(read_status(READ_ONE, &status));

        if (status & mask) { // busy bit set, we block
            ret = RET_SUCCESS; // it's busy and takes a while, so we are confident it's actually clearing
        } else {
            swprint("#ORG#Chip clear not executed while busy\n");
            ret = RET_ERROR; // it's not busy, so we probably tried to clear a locked chip
        }

        RESET();
        return ret;
    }

    RetType lock() override {
        RESUME();

        uint8_t reg;

        CHECK_CALL(block_if_busy());
        CHECK_CALL(read_status(READ_THREE, &reg));

        swprintf("Read S3: 0x%02X\n", reg);
        reg |= ((uint8_t) WPS); // Add the WPS bit
        swprintf("Writing S3 for lock: 0x%02X\n", reg);

        CHECK_CALL(write_status(WRITE_THREE, reg));

        RESET();
        return RET_SUCCESS;
    }

    RetType unlock() override {
        RESUME();

        uint8_t reg;
        CHECK_CALL(block_if_busy());
        CHECK_CALL(read_status(READ_THREE, &reg));

        swprintf("Read S3: 0x%02X\n", reg);
        reg &= !((uint8_t) WPS); // Remove the WPS bit
        swprintf("Writing S3 for unlock: 0x%02X\n", reg);

        CHECK_CALL(write_status(WRITE_THREE, reg));

        RESET();
        return RET_SUCCESS;
    }

    size_t get_block_size() override {
        return page_size;
    }

    size_t get_num_blocks() override {
        return page_count;
    }

    // TODO: If the reentrant statement is the only one, I don't think it needs a CALL, but I could be wrong

    /// @brief Not implemented
    RetType obtain() override {
        return RET_SUCCESS;
    }

    /// @brief Not implemented
    RetType release() override {
        return RET_SUCCESS;
    }

    /* @brief Reads "busy" register, unblocks task when device is not busy
     * @return Always succeeds
     */
    RetType poll() override {
        uint8_t status;
        uint8_t mask = (uint8_t) S_BUSY;

        RESUME();
        if (TID_UNBLOCKED != m_blocked) { // if a task is blocked on this device

            CHECK_CALL(read_status(READ_ONE, &status)); // is the chip still busy?

            if (0 == (status & mask)) { // if not, unblock the blocked task
                WAKE(m_blocked);
                m_blocked = TID_UNBLOCKED;
//                swprint("#GRN#Unblocking!\n");
            }
        }
        RESET();
        return RET_SUCCESS;
    }

//private:
    const uint8_t DUMMY_BYTE = 0xA5;
    const tid_t TID_UNBLOCKED = -1;

    const uint32_t CS_ACTIVE = 0U;
    const uint32_t CS_INACTIVE = 1U;

    enum lock_command_t {
        SECTOR_LOCK = 0x36,
        SECTOR_UNLOCK = 0x39,
    };

    enum read_command_t {
        READ_DATA = 0x03,
    };

    enum program_command_t {
        PAGE_PROGRAM = 0x02,
        SECURITY_PROGRAM = 0x42,
    };

    enum erase_command_t {
        SECTOR_ERASE = 0x20,
        BLOCK_32_ERASE = 0x52,
        BLOCK_64_ERASE = 0xD8,

        CHIP_ERASE = 0xC7, // Can also be 0x60
        SUSPEND_ERASE = 0x75,
        RESUME_ERASE = 0x7A,
    };

    enum write_set_t {
        WRITE_ENABLE = 0x06,
        WRITE_ENABLE_VOLTATILE = 0x50,
        WRITE_DISABLE = 0x04,
    };

    enum read_status_t {
        READ_ONE = 0x05,
        READ_TWO = 0x35,
        READ_THREE = 0x15,
    };

    enum write_status_t {
        WRITE_ONE = 0x01,
        WRITE_TWO = 0x31,
        WRITE_THREE = 0x11,
    };

    enum status_mask_t {
        S_SRP = 1 << 7,
        S_SEC = 1 << 6,
        S_TB = 1 << 5,
        S_BP2 = 1 << 4,
        S_BP1 = 1 << 3,
        S_BP0 = 1 << 2,
        S_WEL = 1 << 1,
        S_BUSY = 1 << 0,

        SUS = 1 << 7,
        CMP = 1 << 6,
        LB3 = 1 << 5,
        LB2 = 1 << 4,
        LB1 = 1 << 3,
//        R = 2 << 2, // Reserved
        QE = 1 << 1,
        SRL = 1 << 0,

        HOLD = 1 << 7,
        DRV_1 = 1 << 6,
        DRV_0 = 1 << 5,
//        R = 1 << 4, // Reserved
//        R = 1 << 3, // Reserved
        WPS = 1 << 2,
//        R = 1 << 1, // Reserved
//        R = 1 << 0, // Reserved
    };

    // device could be busy doing a write op, need to have a block/unblock mechanism
    tid_t m_blocked = TID_UNBLOCKED; // the task blocked on this device
    BlockingSemaphore m_lock;

    // peripherals
    SPIDevice &m_spi;
    GPIODevice &m_cs;
    uint8_t m_write_buf[W25Q_BUF_SIZE];
    uint8_t m_read_buf[W25Q_BUF_SIZE];

    // device properties
    uint32_t m_dev_id = 0;
    size_t page_size = 0; // unit of bytes
    size_t page_count = 0;

    /**
     * @brief uses a read operation to check the BUSY register
     * stores the currently dispatched task in m_blocked and blocks if the mem is busy
     * @return  RET_ERROR if there is already a blocked task or the register read fails
     * @return 	RET_SUCCESS otherwise
     */
    RetType block_if_busy() {
        uint8_t status;
        uint8_t mask = (uint8_t) S_BUSY;

        RESUME();
        if (TID_UNBLOCKED != m_blocked) {
            // the blocked ID has been changed from default, so someone else is using this
            RESET();
            return RET_ERROR;
        }

        CHECK_CALL(read_status(READ_ONE, &status));

        if (status & mask) { // busy bit set, we block
//            swprint("#ORG#Blocking!\n");
            m_blocked = sched_dispatched;
            BLOCK();
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType read_id(uint32_t* id) {
        uint8_t cmd_bytes[1] = {0x9F};
        uint8_t id_bytes[3] = {0x00, 0x00, 0x00};

        RESUME();

        CHECK_CALL(m_spi_wr(cmd_bytes, sizeof(cmd_bytes), id_bytes, sizeof(id_bytes)));

        *id = (id_bytes[0] << 16) + (id_bytes[1] << 8) + id_bytes[2];

        RESET();
        return RET_SUCCESS;
    }

    RetType read_status(read_status_t cmd, uint8_t* dst) {
        RESUME();

        uint8_t cmd_byte;
        cmd_byte = (uint8_t) cmd;
        CHECK_CALL(m_spi_wr(&cmd_byte, 1, dst, 1));

        RESET();
        return RET_SUCCESS;
    }

    RetType write_status(write_status_t cmd, uint8_t src) {
        RESUME();

        // TODO: Appropriately set volatile status registers
        uint8_t cmd_bytes[2];
        cmd_bytes[0] = (uint8_t) cmd;
        cmd_bytes[1] = src;

        CHECK_CALL(write_enable_set(WRITE_ENABLE)); // Writes require WEL
        CHECK_CALL(m_spi_w(cmd_bytes, sizeof(cmd_bytes)));

        RESET();
        return RET_SUCCESS;
    }

    RetType mem_read(uint32_t addr, uint8_t* dst, size_t len) {
        uint8_t cmd_bytes[4];
        cmd_bytes[0] = (uint8_t) READ_DATA;
        addr_to_24(addr, cmd_bytes + 1);

        RESUME();

        CHECK_CALL(block_if_busy());  // Reads are ignored if dev busy
        CHECK_CALL(m_spi_wr(cmd_bytes, sizeof(cmd_bytes), dst, len));

        RESET();
        return RET_SUCCESS;
    }

    RetType mem_program(program_command_t cmd, uint32_t addr, uint8_t* src, size_t len) {
        uint8_t cmd_bytes[4];
        cmd_bytes[0] = (uint8_t) cmd;
        addr_to_24(addr, cmd_bytes + 1);

        RESUME();

        CHECK_CALL(write_enable_set(WRITE_ENABLE)); // Writes require WEL
        CHECK_CALL(m_spi_ww(cmd_bytes, sizeof(cmd_bytes), src, len));

        RESET();
        return RET_SUCCESS;
    }

    RetType write_enable_set(write_set_t mode) {
        uint8_t cmd = (uint8_t) mode;
        RESUME();

        CHECK_CALL(block_if_busy());
        CHECK_CALL(m_spi_w(&cmd, 1));

        RESET();
        return RET_SUCCESS;
    }

    RetType erase(erase_command_t cmd, uint32_t addr) {
        uint8_t cmd_bytes[4];
        cmd_bytes[0] = (uint8_t) cmd;
        addr_to_24(addr, cmd_bytes + 1);
        size_t cmd_len = sizeof(cmd_bytes);

        RESUME();

        CHECK_CALL(write_enable_set(WRITE_ENABLE));
        if (CHIP_ERASE == cmd) {
            cmd_len = 1;
        }
        CHECK_CALL(m_spi_w(cmd_bytes, cmd_len));

        RESET();
        return RET_SUCCESS;
    }

/// Common read/write patterns
/// NOTE: They do not acquire or release the lock
private:
    RetType m_spi_w(uint8_t* buf, size_t len) {
        RESUME();
        CALL(m_lock.acquire());

        CHECK_CALL(m_cs.set(CS_ACTIVE));
        CHECK_CALL(m_spi.write(buf, len));
        CHECK_CALL(m_cs.set(CS_INACTIVE));

        CALL(m_lock.release());
        RESET();
        return RET_SUCCESS;
    }

    RetType m_spi_ww(uint8_t* buf1, size_t len1, uint8_t* buf2, size_t len2) {
        RESUME();
        CALL(m_lock.acquire());

        if ((len1 + len2) > W25Q_BUF_SIZE) {
            RESET();
            return RET_ERROR;
        }

        memcpy(m_write_buf, buf1, len1);
        memcpy(m_write_buf + len1, buf2, len2);

        CHECK_CALL(m_cs.set(CS_ACTIVE));
        CHECK_CALL(m_spi.write(m_write_buf, len1 + len2));
        CHECK_CALL(m_cs.set(CS_INACTIVE));

        CALL(m_lock.release());
        RESET();
        return RET_SUCCESS;
    }

    RetType m_spi_wr(uint8_t* buf1, size_t len1, uint8_t* buf2, size_t len2) {
        RESUME();
        CALL(m_lock.acquire());
        if ((len1 + len2) > W25Q_BUF_SIZE) {
            RESET();
            return RET_ERROR;
        }

        memcpy(m_write_buf, buf1, len1);

        CHECK_CALL(m_cs.set(CS_ACTIVE));
        CHECK_CALL(m_spi.write_read(m_write_buf, m_read_buf, len1 + len2));
        CHECK_CALL(m_cs.set(CS_INACTIVE));

        memcpy(buf2, m_read_buf + len1, len2);

        CALL(m_lock.release());
        RESET();
        return RET_SUCCESS;
    }

    void addr_to_24(uint32_t addr, uint8_t* dest) {
        dest[0] = (uint8_t) ((addr & 0x00FF0000U) >> 16);
        dest[1] = (uint8_t) ((addr & 0x0000FF00U) >> 8);
        dest[2] = (uint8_t) (addr & 0x000000FFU);
    }
};

#undef CHECK_CALL
#endif //LAUNCH_CORE_W25Q_H
