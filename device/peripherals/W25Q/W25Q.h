/**
 * Platform Independent Implementation for a W25Q128JVSIQ NOR-flash device
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


// for convenience
#define CHECK_CALL(expr) {if (RET_ERROR == CALL(expr)) {RESET(); return RET_ERROR;}}
// convert 32-bit integer to 24-bit address, bit shifts make sure the result is big endian
#define ADDR_TO_24(addr) (uint8_t) (((addr) >> 16) & 0xFFU), \
    (uint8_t) (((addr) >> 8) & 0xFFU), \
    (uint8_t) ((addr) & 0xFFU)
#define PAGE_TO_SEC(addr) ((addr / 16) * 16)

// chip information
#define W25Q128JV_JEDEC_ID 0x00EF4018
#define W25Q_MAX_CMD_SIZE 8 // maximum length of a command
#define W25Q_MAX_DATA_SIZE 256 // the largest quantity of data we could want
#define W25Q_BUF_SIZE (W25Q_MAX_CMD_SIZE + W25Q_MAX_DATA_SIZE)

class W25Q : public BlockDevice {
public:

    W25Q(const char* name, SPIDevice& spi, GPIODevice& cs, GPIODevice& activity) :
            BlockDevice(name), m_spi(spi), m_cs(cs), m_inner_lock(1), m_lock(1), m_activity(activity) {}

    RetType init() override{
        RESUME();
        static RetType ret;

        CHECK_CALL(read_id(&m_dev_id));
        swprintf("JEDEC ID: 0x%06X\n", m_dev_id);

        if (W25Q128JV_JEDEC_ID == m_dev_id) {
            m_page_count = 0x10000;
            m_buffer_location = 0xFFF0;

            ret = RET_SUCCESS;
        } else {
            swprintf("Incorrect JEDEC ID, expected: 0x%06X\n", W25Q128JV_JEDEC_ID);
            ret = RET_ERROR;
        }

        RESET();
        return ret;
    }

    RetType write(size_t block, uint8_t *buff) override {
        static uint8_t temp[W25Q_MAX_DATA_SIZE];
        bool needs_overwrite;

        RESUME();

        if (block > m_page_count) {
            return RET_ERROR;
        };

        needs_overwrite = false;
        CHECK_CALL(read_page(block, temp));
        for (size_t i = 0; i < m_page_size; i++) {
            if (0xFFU != temp[i]) { // is a cleared byte?
                needs_overwrite = true; // this page or sector have content, we need to erase in order to write
                printf("Non-cleared byte '%c' at page 0x%04X, byte 0x%02X\n", (char) temp[i], block, i);
                break;
            }
        }

        if (needs_overwrite) {
            static size_t sector_start = page_to_sector_start(block);
            static size_t offset = block - sector_start;

            printf("Sector starting at 0x%06X needs to be overwritten\n", sector_start);
            //
//            printf("Clearing buffer sector. 0x%06X..\n", m_buffer_location);
            CHECK_CALL(erase_sector(m_buffer_location));

            if (offset > 0) {
//                printf("Copying first bytes...\n");
                CHECK_CALL(copy_pages(m_buffer_location, sector_start, offset));
            }

            // add the new block where it should be
//            printf("Writing new data to 0x%06X\n", m_buffer_location + offset);
            CHECK_CALL(write_page(m_buffer_location + offset, buff));

            // copy existing blocks after new block
            if (offset < (m_sector_size - 1)) { // are there any blocks after the new one?
                CHECK_CALL(copy_pages(m_buffer_location + offset + 1, block + 1, m_sector_size - offset - 1))
            }

            printf("Erasing target sector...\n");
            CHECK_CALL(erase_sector(sector_start));

            printf("Copying to target sector\n");
            CHECK_CALL(copy_pages(sector_start, m_buffer_location, m_sector_size));
        } else {
            printf("No overwrite needed");
            CHECK_CALL(write_page(block, buff));
        }

        RESET();
        return RET_SUCCESS;
    }

    /**
     * Read a block from the device (wraps read_page)
     * @param block
     * @param buff
     * @return
     */
    RetType read(size_t block, uint8_t *buff) override {
        return read_page(block, buff);
    }

    RetType clear() override {
        return erase_region(CHIP_ERASE, 0);
    }

    RetType lock() override {
        RESUME();

        uint8_t reg;

        CHECK_CALL(await());
        CHECK_CALL(read_status(READ_THREE, &reg));

        reg |= ((uint8_t) WPS); // Add the WPS bit

        CHECK_CALL(write_status(WRITE_THREE, reg));

        RESET();
        return RET_SUCCESS;
    }

    RetType unlock() override {
        RESUME();

        uint8_t reg;
        CHECK_CALL(await());
        CHECK_CALL(read_status(READ_THREE, &reg));

        reg &= !((uint8_t) WPS); // Remove the WPS bit

        CHECK_CALL(write_status(WRITE_THREE, reg));

        RESET();
        return RET_SUCCESS;
    }

    size_t get_block_size() override {
        return m_page_size;
    }

    size_t get_num_blocks() override {
        return m_page_count;
    }

    RetType obtain() override {
        return m_lock.acquire();
    }

    RetType release() override {
        return m_lock.release();
    }

    /**
     * @brief Reads "busy" register, unblocks task when device is not busy
     * @return Always succeeds
     */
    RetType poll() override {
        uint8_t status;
        uint8_t mask = (uint8_t) S_BUSY; // TODO: make read_status and write_status take an optional mask argument

        RESUME();
        if (TID_UNBLOCKED != m_blocked) { // if a task is blocked on this device

            CHECK_CALL(read_status(READ_ONE, &status)); // is the chip still busy?

            if (0 == (status & mask)) { // if not, unblock the blocked task
                WAKE(m_blocked);
                m_blocked = TID_UNBLOCKED;
                m_activity.set(0);
            }
        }
        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief uses a read operation to check the BUSY register
     * if busy, stores the currently dispatched task in m_blocked, sets m_activity high, BLOCKS
     * @return  RET_ERROR if there is already a blocked task or the register read fails
     * @return 	RET_SUCCESS otherwise
     */
    RetType await() {
        uint8_t status;
        uint8_t mask = (uint8_t) S_BUSY;
        RESUME();
        // TODO: Use a queue instead of turning away subsequent requests
        if (TID_UNBLOCKED != m_blocked) {
            // the blocked ID has been changed from default, so someone else is using this already
            return RET_ERROR;
        }

        CHECK_CALL(read_status(READ_ONE, &status));

        if (status & mask) { // busy bit set, we block
            m_activity.set(1);
            m_blocked = sched_dispatched;
            BLOCK();
        }

        RESET();
        return RET_SUCCESS;
    }

public:
    /// COMMANDS AND CONSTANTS
    const uint8_t DUMMY_BYTE = 0xA5;
    const tid_t TID_UNBLOCKED = -1;

    const uint32_t CS_ACTIVE = 0U;
    const uint32_t CS_INACTIVE = 1U;

    enum reset_command_t {
        ENABLE_RESET = 0x66,
        CHIP_RESET = 0x99,
    };

    enum lock_command_t {
        SECTOR_LOCK = 0x36,
        SECTOR_UNLOCK = 0x39,
    };

    enum read_command_t {
        READ_DATA = 0x03U,
        READ_SECURITY = 0x48U
    };

    enum program_command_t {
        PAGE_PROGRAM = 0x02U,
        SECURITY_PROGRAM = 0x42U,
    };

    enum erase_command_t {
        SECTOR_ERASE = 0x20U,
        BLOCK_32_ERASE = 0x52U,
        BLOCK_64_ERASE = 0xD8U,
        CHIP_ERASE = 0xC7U, // Can also be 0x60
    };

    enum erase_ctl_t {
        SUSPEND_ERASE = 0x75,
        RESUME_ERASE = 0x7A,
    };

    enum write_set_t {
        WRITE_ENABLE = 0x06,
        WRITE_ENABLE_VOLATILE = 0x50,
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
        QE = 1 << 1,
        SRL = 1 << 0,

        HOLD = 1 << 7,
        DRV_1 = 1 << 6,
        DRV_0 = 1 << 5,
        WPS = 1 << 2,
    };

    /* Higher-level helpers ################################################################################ */

    /**
     * Write an entire block
     * @param page  page to write to
     * @param buff  data to write (must be at least one page)
     * @return
     */
    RetType write_page(size_t page, uint8_t* buff) {
        return write_mem(page * m_page_size, buff, m_page_size);
    }

    /**
     * Read an entire block
     * @param page  page to read
     * @param buff  buffer to read into (must be at least one page)
     * @return
     */
    RetType read_page(size_t page, uint8_t* buff) {
        return read_mem(page * m_page_size, buff, m_page_size);
    }

    /**
     * Copy <len> blocks from <src> to <dest>
     * Users should acquire() and release() this device to prevent interruption
     * @param dest block to start copying to
     * @param dst block to start copying from
     * @param len how many blocks to copy
     * @return
     */
    RetType copy_pages(size_t dest, size_t src, size_t len) {
        RESUME();
        static uint8_t temp[W25Q_MAX_DATA_SIZE];
        static size_t i;

        for (i = 0; i < len; i++) {
//            printf("Copying page 0x%04X to 0x%04X, %ld of %ld...\n",
//                   src + i, dest + i, i + 1, len);
            CHECK_CALL(read_page(src + i, temp));
            CHECK_CALL(write_page(dest + i, temp));
        }

        RESET();
        return RET_SUCCESS;
    }
/**
     * Set the Write Enable bit before modifying chip memory
     * @param mode WRITE_ENABLE or WRITE_DISABLE
     * @return
     */
    RetType write_enable_set(write_set_t mode) {
        uint8_t cmd = (uint8_t) mode;
        return m_spi_w(&cmd, 1);
    }
    /**
     * Erase a sector (wraps erase_region)
     * @param block A block in the target sector
     * @return
     */
    RetType erase_sector(size_t block) {
        return erase_region(SECTOR_ERASE, block * m_page_size);
    }

    /// LOW-LEVEL INSTRUCTIONS

    // device properties
    uint32_t m_dev_id = 0;
    static const size_t m_page_size = 256;         // how big are pages (bytes)
    static const size_t m_sector_size = 16;       // how big are sectors (blocks)
    size_t m_page_count = 0;        // how many blocks
    size_t m_buffer_location = 0;   // where do we write/erase to?

    RetType read_id(uint32_t* id) {
        uint8_t cmd_bytes[1] = {0x9F};
        uint8_t id_bytes[3] = {DUMMY_BYTE};

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
        uint8_t cmd_bytes[2] = {(uint8_t) cmd, src};
        RetType ret;

        RESUME();

        // TODO: Appropriately set volatile status registers

        CHECK_CALL(await());
        CHECK_CALL(write_enable_set(WRITE_ENABLE)); // Writes require WEL
        CHECK_CALL(m_spi_w(cmd_bytes, sizeof(cmd_bytes)));

        RESET();
        return RET_SUCCESS;
    }

    RetType read_mem(uint32_t addr, uint8_t* dst, size_t len) {
        uint8_t cmd_bytes[4] = {(uint8_t) READ_DATA, ADDR_TO_24(addr)};

        RESUME();

        CHECK_CALL(await());
        CHECK_CALL(m_spi_wr(cmd_bytes, sizeof(cmd_bytes), dst, len));

        RESET();
        return RET_SUCCESS;
    }

    RetType write_mem(uint32_t addr, uint8_t* src, size_t len) {
        uint8_t cmd_bytes[4] = {(uint8_t) PAGE_PROGRAM, ADDR_TO_24(addr)};
        RESUME();
        // basic validation
        if (addr > (m_page_size * m_page_count)) { // in memory bounds
            return RET_ERROR;
        }
        if (((addr + len - 1) / m_page_size) != (addr / m_page_size)) { // can't cross page boundary
            return RET_ERROR;
        }

        CHECK_CALL(await());
        CHECK_CALL(write_enable_set(WRITE_ENABLE)); // Writes require WEL
        CHECK_CALL(m_spi_ww(cmd_bytes, sizeof(cmd_bytes), src, len));

        RESET();
        return RET_SUCCESS;
    }

    /**
     * Read security registers
     * @param src Byte to start reading from
     * @param dst Buffer to read into
     * @param len How much to read
     * @return
     */
    RetType read_security(uint32_t src, uint8_t* dst, size_t len) {
        RESUME();
        if (src + len > 256*3) { // in bounds
            return RET_ERROR;
        }

        static uint8_t reg = ((src / 256) + 1) << 4; // which register 1-3 -> 0x10 - 0x30
        static uint8_t byte = src % 256; // which byte in the register
        static uint8_t cmd[4] = {(uint8_t) READ_SECURITY, 0x00U, reg, byte};

        CHECK_CALL(await());
        CHECK_CALL(write_enable_set(WRITE_ENABLE)); // Writes require WEL
        CHECK_CALL(m_spi_wr(cmd, sizeof(cmd), dst, len));

        RESET();
        return RET_SUCCESS;
    }

    /**
     * Program security registers
     * @param dst Byte to write into
     * @param src Buffer to write from
     * @param len How much to write
     * NOTE: may not cross a 256-byte page boundary
     * @return
     */
    RetType write_security(uint32_t dst, uint8_t* src, size_t len) {
        RESUME();

        if (dst + len > 256*3) { // out of bounds
            return RET_ERROR;
        }
        if (((dst + len) / 256) != (dst / 256)) { // can't cross page boundary
            return RET_ERROR;
        }
        static uint8_t reg = ((dst / 256) + 1) << 4; // which register 1-3 -> 0x10 - 0x30
        static uint8_t byte = dst % 256; // which byte in the register
        static uint8_t cmd[4] = {(uint8_t) SECURITY_PROGRAM, 0x00U, reg, byte};

        CHECK_CALL(await());
        CHECK_CALL(m_spi_ww(cmd, sizeof(cmd), src, len));

        RESET();
        return RET_SUCCESS;
    }

    RetType chip_reset() {
        uint8_t en = (uint8_t) ENABLE_RESET;
        uint8_t rst = (uint8_t) CHIP_RESET;
        RESUME();
        CHECK_CALL(m_spi_w(&en, 1));
        SLEEP(1);
        CHECK_CALL(m_spi_w(&rst, 1));
        SLEEP(1);
        RESET();
        return RET_SUCCESS;
    }

    inline size_t page_to_sector_start(size_t page) {
        return (page / m_sector_size) * m_sector_size;
    }

    /**
     * Erase a region of the chip
     * @param cmd   one of SECTOR_ERASE (4kB, 16 blocks), BLOCK_32(64)_ERASE (32 (64) kB, 256 (512) blocks), CHIP_ERASE
     * @param page  an address inside the region to erase
     * @return
     */
    RetType erase_region(erase_command_t cmd, uint32_t addr) {
        RESUME();
        static uint8_t cmd_bytes[4] = {(uint8_t) cmd, ADDR_TO_24(addr)};
        static size_t cmd_len = sizeof(cmd_bytes);

        if (CHIP_ERASE == cmd) {
            cmd_len = 1;
        }
        CHECK_CALL(await());
        CHECK_CALL(write_enable_set(WRITE_ENABLE));
        CHECK_CALL(m_spi_w(cmd_bytes, cmd_len));
        CHECK_CALL(await()); // make sure everyone knows you're busy

        RESET();
        return RET_SUCCESS;
    }

    /*
     * Common read/write patterns
     * These take care of lowering/raising CS and the TransmitReceive workaround for write-then-read
     */

    /* NOTE ON WHY WE NEED THESE FUNCTIONS
     * Transactions with the W25Q consist of a command, then additional clock pulses for a response. From what I can
     * tell, if we use separate m_spi.write/read calls, there is unacceptable delay between the command and the response
     * pulses, so the device does nothing. HAL only allows us to simultaneously read/write using TransmitReceive, which
     * uses one length for both buffers - they have to be equal length.
     *
     * There are two ways to solve this: the old way is to call TransmitReceive with two buffers, each as large
     * as the total size of the largest transaction we use, but the vast majority of the Transmit buffer is wasted space,
     * only there so that TransmitReceive has something to address for the read bytes.
     *
     * What we do now is to use one buffer of that size and give TransmitReceive (buff, buff) for an N-byte
     * command. This gives it enough addressable memory to send the additional pulses and read the response, but once
     * the Transmit part reaches the bytes the Receive part read in N bytes earlier, the controller will begin
     * transmitting again, so this workaround is only acceptable for peripherals that ignore commands after they begin
     * transmitting.
     */

    /// CHIP COMMUNICATION HELPERS ################################################################################

    /**
     * @brief Write to the chip
     * @param buf buffer to write
     * @param len # of bytes to write
     * @return RET_ERROR or RET_SUCCESS
     */
    RetType m_spi_w(uint8_t *buf, size_t len) {
        RESUME();
        RetType ret;

        CHECK_CALL(m_cs.set(CS_ACTIVE));
        ret = CALL(m_spi.write(buf, len));
        CHECK_CALL(m_cs.set(CS_INACTIVE));

        RESET();
        return ret;
    }

    /**
     * @brief Write two buffers to the chip in one transaction
     * @param buf1 first buffer
     * @param len1 length of first buffer
     * @param buf2 second buffer
     * @param len2 length of second buffer
     * @return RET_ERROR or RET_SUCCESS
     */
    RetType m_spi_ww(uint8_t *buf1, size_t len1, uint8_t *buf2, size_t len2) {
        RESUME();
        if ((len1 + len2) > W25Q_BUF_SIZE) {
            return RET_ERROR;
        }
        RetType ret;

        memcpy(m_spi_buf, buf1, len1);
        memcpy(m_spi_buf + len1, buf2, len2);

        CHECK_CALL(m_cs.set(CS_ACTIVE));
        ret = CALL(m_spi.write(m_spi_buf, len1 + len2));
        CHECK_CALL(m_cs.set(CS_INACTIVE));

        RESET();
        return ret;
    }

    /**
     * @brief Write a buffer then read to another in one transaction
     * @param write_buf buffer to write
     * @param write_len bytes to write
     * @param read_buf buffer to read
     * @param read_len bytes to read
     * @return
     */
    RetType m_spi_wr(uint8_t* write_buf, size_t write_len, uint8_t* read_buf, size_t read_len) {
        RESUME();
        if ((write_len + read_len) > W25Q_BUF_SIZE) {
            return RET_ERROR;
        }
        RetType ret;

        memcpy(m_spi_buf, write_buf, write_len); // put the command in the transaction buffer
        memset(m_spi_buf + write_len, 0x00, sizeof(m_spi_buf) - write_len); // clear the rest

        CHECK_CALL(m_cs.set(CS_ACTIVE));
        ret = CALL(m_spi.write_read(m_spi_buf, m_spi_buf, write_len + read_len));
        CHECK_CALL(m_cs.set(CS_INACTIVE));

        if (RET_SUCCESS == ret) {
            memcpy(read_buf, m_spi_buf + write_len, read_len); // put the response where the caller wants it
        }

        RESET();
        return ret;
    }

    // INFRASTRUCTURE ##########################################################
    // used with poll() to make sure something trying to write waits until BUSY
    // is unset
    tid_t m_blocked = TID_UNBLOCKED;

    // make sure SPI transactions don't get interrupted
    BlockingSemaphore m_inner_lock;

    // make sure blocks of transactions don't get interrupted. The inner lock is
    // redundant if this gets used, but not everyone needs to use the outer lock
    BlockingSemaphore m_lock;

    // Peripherals
    SPIDevice& m_spi;
    GPIODevice& m_cs;
    GPIODevice& m_activity;
    uint8_t m_spi_buf[W25Q_BUF_SIZE] = {0};
};

#undef CHECK_CALL
#endif //LAUNCH_CORE_W25Q_H
