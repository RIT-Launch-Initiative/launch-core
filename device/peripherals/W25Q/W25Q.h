/**
 * Platform Independent Implementation for a W25Q Flash Memory Chip
 *
 * @author Aaron Chan, Yevgeniy Gorbachev
 */

#ifndef LAUNCH_CORE_W25Q_H
#define LAUNCH_CORE_W25Q_H

#include "device/SPIDevice.h"
#include "device/GPIODevice.h"
#include "sched/sched.h"
#include "sched/macros.h"
#include "device/BlockDevice.h"

extern void swprint(const char*);
extern int swprintf(const char* fmt, ...);

#define CHECK_CALL(expr) {if (RET_SUCCESS != CALL(expr)) {RESET(); return RET_ERROR;}}

class W25Q : public BlockDevice {
public:

    W25Q(const char* name, SPIDevice &m_spi, GPIODevice &csPin, uint8_t erase = 0) :
            BlockDevice(name), m_spi(m_spi), m_cs(csPin), init_erase(erase) {}

    RetType init() {
        RESUME();

        CHECK_CALL(read_id(&device_id));
        if (device_id != 0x17) {
        	swprintf("Incorrect device ID: 0x%08x\n", device_id);
        	RESET();
        	return RET_ERROR;
        }

        page_size = 0xFF;
        page_count = 0xFFFF;

        if (init_erase) {
        	// TODO: Erase chip
        }

        RESET();
        return RET_SUCCESS;
    }


    RetType write(size_t block, uint8_t *buff) override {
        RESUME();
        if (block > getNumBlocks()) {
        	RESET();
        	return RET_ERROR;
        };

        CHECK_CALL(mem_program(PAGE_PROGRAM, block << 8, buff, getBlockSize()));

        RESET();
        return RET_SUCCESS;
    }

    RetType read(size_t block, uint8_t *buff) override {
        RESUME();
        if (block > getNumBlocks()) {
        	RESET();
        	return RET_ERROR;
        };

        CHECK_CALL(mem_read(block << 8, buff, getBlockSize()));

        RESET();
        return RET_SUCCESS;
    }


    size_t getBlockSize() override {
        return page_size;
    }

    size_t getNumBlocks() override {
        return page_count;
    }

    // Might want to implement this at some point
    // Many async calls between a WEL and the actual write, so this could be useful
    // to prevent interruption
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
    	RESUME();
    	if (TID_UNBLOCKED != m_blocked) { // if a task is blocked on this device
			uint8_t status;
			uint8_t mask = (uint8_t) S_BUSY;

			CHECK_CALL(read_status(READ_ONE, &status)); // is the chip still busy?

			if (0 == (status & mask)) { // if not, unblock the blocked task
				m_blocked = TID_UNBLOCKED;
				WAKE(m_blocked);
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

	enum read_command_t {
		READ_DATA = 0x03,
//		FAST_READ = 0x00,
//
//		FAST_READ_DUAL = 0x03,
//		FAST_READ_QUAD = 0x06,
//
//		FAST_READ_DUAL_IO = 0x0B,
//		FAST_READ_QUAD_IO = 0x0E,
//
//		WORD_READ_QUAD_IO = 0xE7,
//		OCTAL_WORD_READ_QUAD_IO = 0xE3,
	};

	enum program_command_t {
		PAGE_PROGRAM = 0x02,
//		QUAD_PAGE_PROGRAM = 0x32
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
	};

    // device could be busy doing a write op, need to have a block/unblock mechanism
    tid_t m_blocked = TID_UNBLOCKED; // the task blocked on this device

    // necessary peripherals
	SPIDevice &m_spi;
    GPIODevice &m_cs;

    // device properties
    uint8_t init_erase = 0; // erase on startup
    uint32_t device_id = 0;
    size_t page_size = 0; // unit of bytes
    size_t page_count = 0;

    /* @brief uses a read operation to check the BUSY register
     * stores the currently dispatched task in m_blocked and blocks if the mem is busy
     * @return if there is already a task blocked on this device, RET_ERROR
     * 			if register read does not succeed, its return value
     * 			if device is busy flushing, RET_BLOCKED
     * 			otherwise RET_SUCCESS
     */
    RetType block_if_busy() {
    	RESUME();
    	if (TID_UNBLOCKED != m_blocked) {
    		// the blocked ID has been changed from default, so someone else is using this
    		RESET();
    		return RET_ERROR;
    	}

    	uint8_t status;
    	uint8_t mask = (uint8_t) S_BUSY;

    	CHECK_CALL(read_status(READ_ONE, &status));

    	if (status & mask) { // busy bit set, we block
    		m_blocked = sched_dispatched;
    		BLOCK();
    	}

    	RESET();
    	return RET_SUCCESS;
    }

    RetType read_id(uint32_t* id) {
        RESUME();
        if (nullptr == id) {
        	RESET();
        	return RET_ERROR;
        }

        uint8_t cmd_bytes[1] = {0x9F};
    	uint8_t id_bytes[3] = {0xFF, 0xFF, 0xFF};

    	CHECK_CALL(m_cs.set(CS_ACTIVE));
    	CHECK_CALL(m_spi.write(cmd_bytes, sizeof(cmd_bytes)));
    	CHECK_CALL(m_spi.read(id_bytes, sizeof(id_bytes)));
    	CHECK_CALL(m_cs.set(CS_INACTIVE));

    	uint32_t result = (id_bytes[0] << 16) + (id_bytes[1] << 8) + id_bytes[2];
    	swprintf("Read ID: 0x%06x\n", result);
    	*id = result;

        RESET();
        return RET_SUCCESS;
    }

    RetType read_status(read_status_t cmd, uint8_t* dst) {
    	RESUME();
    	if (nullptr == dst) {
    		RESET();
    		return RET_ERROR;
    	}

    	uint8_t cmd_bytes[1] = {(uint8_t) cmd};
    	uint8_t cmd_byte = (uint8_t) cmd;

    	CHECK_CALL(m_cs.set(CS_ACTIVE));
    	CHECK_CALL(m_spi.write(&cmd_byte, 1));
    	CHECK_CALL(m_spi.read(dst, 1));
    	CHECK_CALL(m_cs.set(CS_INACTIVE));

    	RESET();
    	return RET_SUCCESS;
    }

    RetType write_status(write_status_t cmd, uint8_t src) {
    	RESUME();

    	uint8_t cmd_bytes[2] = {(uint8_t) cmd, src};

    	// TODO: Appropriately set volatile status registers
    	CHECK_CALL(write_enable_set(WRITE_ENABLE));

    	CHECK_CALL(m_cs.set(CS_ACTIVE));
    	CHECK_CALL(m_spi.write(cmd_bytes, sizeof(cmd_bytes)));
    	CHECK_CALL(m_cs.set(CS_INACTIVE));

    	RESET();
    	return RET_SUCCESS;
    }

    RetType mem_read(uint32_t addr, uint8_t* dst, size_t len) {
    	RESUME();
    	if ((nullptr == dst) || (0 >= len)) {
    		RESET();
    		return RET_ERROR;
    	}

    	CHECK_CALL(block_if_busy());

    	// TODO: Add extra speed modes

    	read_command_t cmd = READ_DATA;
    	uint8_t cmd_bytes[4];
    	cmd_bytes[0] = (uint8_t) cmd;
    	addr_to_24(addr, cmd_bytes + 1);

    	CHECK_CALL(m_cs.set(CS_ACTIVE));
    	CHECK_CALL(m_spi.write(cmd_bytes, sizeof(cmd_bytes)));
    	CHECK_CALL(m_spi.read(dst, len));
    	CHECK_CALL(m_cs.set(CS_INACTIVE));

    	RESET();
    	return RET_SUCCESS;
    }

    RetType mem_program(program_command_t cmd, uint32_t addr, uint8_t* src, size_t len) {
    	RESUME();
    	if ((nullptr == src) || (0 >= len)) {
    		RESET();
    		return RET_ERROR;
    	}

    	CHECK_CALL(block_if_busy());
    	CHECK_CALL(write_enable_set(WRITE_ENABLE));
    	// TODO: Add extra speed modes

    	uint8_t cmd_bytes[4];
    	cmd_bytes[0] = (uint8_t) cmd;
    	addr_to_24(addr, cmd_bytes + 1);

    	CHECK_CALL(m_cs.set(CS_ACTIVE));
    	CHECK_CALL(m_spi.write(cmd_bytes, sizeof(cmd_bytes)));
    	CHECK_CALL(m_spi.write(src, len));
    	CHECK_CALL(m_cs.set(CS_INACTIVE));

    	RESET();
    	return RET_SUCCESS;
    }

    RetType write_enable_set(write_set_t mode) {
    	RESUME();

    	uint8_t cmd = (uint8_t) mode;

    	CHECK_CALL(m_cs.set(CS_ACTIVE));
    	CHECK_CALL(m_spi.write(&cmd, 1));
    	CHECK_CALL(m_cs.set(CS_INACTIVE));

    	RESET();
    	return RET_SUCCESS;
    }

    RetType erase(erase_command_t cmd, uint32_t addr, uint8_t force = 0) {
    	RESUME();

    	// for ERASE commands, device can't be busy and
    	CHECK_CALL(block_if_busy());
    	CHECK_CALL(write_enable_set(WRITE_ENABLE));

    	uint8_t cmd_bytes[4];
    	cmd_bytes[0] = (uint8_t) cmd;
    	addr_to_24(addr, cmd_bytes + 1);
    	size_t cmd_len = sizeof(cmd_bytes);

    	if (CHIP_ERASE == cmd) {
    		cmd_len = 1;
    	}

    	CHECK_CALL(m_cs.set(CS_ACTIVE));
    	if (force) {
    		// TODO: Disable all protections
    	}
    	CHECK_CALL(m_spi.write(cmd_bytes, cmd_len));
    	CHECK_CALL(m_cs.set(CS_INACTIVE));

    	RESET();
    	return RET_SUCCESS;
    }
	// TODO: Inline this once the normal version works
    void addr_to_24(uint32_t addr, uint8_t* dest) {
    	dest[0] = (uint8_t) ((addr & 0x00FF0000U) >> 16);
    	dest[1] = (uint8_t) ((addr & 0x0000FF00U) >> 8);
    	dest[2] = (uint8_t) (addr & 0x000000FFU);
    }
};

#endif //LAUNCH_CORE_W25Q_H
