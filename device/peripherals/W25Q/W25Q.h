/**
 * Platform Independent Implementation for a W25Q Flash Memory Chip
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_W25Q_H
#define LAUNCH_CORE_W25Q_H

#include "device/SPIDevice.h"
#include "device/GPIODevice.h"
#include "sched/macros.h"
#include "device/BlockDevice.h"


#define WQ25Q_CMD(command) { \
    chipSelectPin.set(0); \
    command; \
    chipSelectPin.set(1); \
}


#define RET_CHECK(ret) {if (ret != RET_SUCCESS) {RESET(); return ret;}}

// TODO: Might be a better and the actually correct way of doing 32 bit addresses?
#define ADDR_CMD(address) { \
    dataInPin.set((address & 0xFF000000) >> 16); \
    dataInPin.set((address & 0xFF00) >> 8); \
    dataInPin.set((address & 0xFF)); \
}

typedef enum {
    READ_DATA = 0x03,
    FAST_READ = 0x00,

    FAST_READ_DUAL = 0x03,
    FAST_READ_QUAD = 0x06,

    FAST_READ_DUAL_IO = 0x0B,
    FAST_READ_QUAD_IO = 0x0E,

    WORD_READ_QUAD_IO = 0xE7,
    OCTAL_WORD_READ_QUAD_IO = 0xE3,
} READ_COMMAND_T;

typedef enum {
    PAGE_PROGRAM = 0x02,
    QUAD_PAGE_PROGRAM = 0x32
} PROGRAM_COMMAND_T;

typedef enum {
    SECTOR_ERASE = 0x20,
    BLOCK_32_ERASE = 0x52,
    BLOCK_64_ERASE = 0xD8,

    CHIP_ERASE = 0xC7, // Can also be 0x60
    SUSPEND_ERASE = 0x75,
    RESUME_ERASE = 0x7A,
} ERASE_COMMAND_T;

typedef enum {
    WRITE_SET_ENABLE = 0x06,
    WRITE_SET_ENABLE_VOLATILE = 0x50,
    WRITE_SET_DISABLE = 0x04,
} WRITE_SET_T;

typedef enum {
    REGISTER_ONE_READ = 0x05,
    REGISTER_TWO_READ = 0x35,
    REGISTER_THREE_READ = 0x15,
} READ_STATUS_REGISTER_T;

typedef enum {
    REGISTER_ONE_WRITE = 0x01,
    REGISTER_TWO_WRITE = 0x31,
    REGISTER_THREE_WRITE = 0x11,
} WRITE_STATUS_REGISTER_T;

class W25Q : public BlockDevice {
public:
    const uint8_t DUMMY_BYTE = 0xA5;

    W25Q(SPIDevice &spiDevice, GPIODevice &csPin) :
            BlockDevice("FlashDevice"), m_spi(spiDevice), m_cs(csPin) {}


    RetType init() {
        RESUME();
        static uint32_t deviceID = 0;

        RetType ret = CALL(readID(&deviceID));
        if (ret != RET_SUCCESS) return ret;

        switch (deviceID & 0x000000FF) {
            case 0x18: // W25Q128FV
                this->block_count = 128;
                break;
            default: // Currently only using W25Q128FV. Can add more in the future
                return RET_ERROR;
        }

        page_size = 256;
        sector_size = 0x1000;
        sector_count = block_count * 16;
        page_count = (sector_count * sector_size) / page_size;
        block_size = sector_size * 16;
        kb_capacity = (sector_count * sector_size) / 1024;

        ret = CALL(readRegister(REGISTER_ONE_READ, &status_reg_one));
        RET_CHECK(ret);

        ret = CALL(readRegister(REGISTER_TWO_READ, &status_reg_two));
        RET_CHECK(ret);

        ret = CALL(readRegister(REGISTER_THREE_READ, &status_reg_three));
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    RetType toggleWrite(WRITE_SET_T command) {
        RESUME();
        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret)

        ret = CALL(m_spi.write(reinterpret_cast<uint8_t *>(&command), 1));
        RET_CHECK(ret)

        ret = CALL(m_cs.set(1));
        RET_CHECK(ret)

        RESET();
        return ret;
    }

    RetType write(size_t block, uint8_t *buff) override {
        RESUME();

        RetType ret = CALL(write_block(buff, block_size, block, 0));
        RET_CHECK(ret);

        RESET();
        return ret;
    }

    RetType read(size_t block, uint8_t *buff) override {
        RESUME();

        RetType ret = CALL(read_block(buff, block_size, block, 0));
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        return RET_SUCCESS;
    }

    size_t getBlockSize() {
        return block_size;
    }

    size_t getNumBlocks() {
        return block_count;
    }

private:
    SPIDevice &m_spi;
    GPIODevice &m_cs;
//    GPIODevice &clockPin;

    uint16_t page_size;
    uint32_t page_count;
    uint32_t sector_size;
    uint32_t sector_count;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t kb_capacity;
    uint8_t status_reg_one;
    uint8_t status_reg_two;
    uint8_t status_reg_three;
    uint8_t tx_buff[8];


    RetType readID(uint32_t *deviceID) {
        RESUME();

        static uint8_t read_buff[4] = {};

        tx_buff[0] = 0x9F;
        tx_buff[1] = 0xA5;
        tx_buff[2] = 0xA5;
        tx_buff[3] = 0xA5;

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        ret = CALL(m_spi.write_read(tx_buff, read_buff, 4));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RET_CHECK(ret);

        *deviceID = (read_buff[0] << 24) | (read_buff[1] << 16) | (read_buff[2] << 8) | read_buff[3];

        RESET();
        return RET_SUCCESS;
    }

    size_t pageToSector(size_t pageAddr) {
        return (pageAddr * this->page_size) / this->sector_size;
    }

    size_t sectorToPage(size_t sectorAddr) {
        return (sectorAddr * this->sector_size) / this->page_size;
    }

    size_t pageToBlock(size_t pageAddr) {
        return (pageAddr * this->page_size) / this->block_size;
    }

    size_t blockToPage(size_t blockAddr) {
        return (blockAddr * this->block_size) / this->page_size;
    }

    size_t sectorToBlock(size_t sectorAddr) {
        return (sectorAddr * this->sector_size) / this->block_size;
    }

    size_t blockToSector(size_t blockAddr) {
        return (blockAddr * this->block_size) / this->sector_size;
    }

    RetType readRegister(READ_STATUS_REGISTER_T reg, uint8_t *receiveBuff) {
        RESUME();

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret)

        ret = m_spi.write(reinterpret_cast<uint8_t *>(&reg), 1);
        RET_CHECK(ret)

        ret = m_cs.set(1);
        RET_CHECK(ret)

        ret = CALL(m_spi.read(receiveBuff, 1));
        RET_CHECK(ret)

        RESET();
        return ret;
    }

    RetType writeRegister(WRITE_STATUS_REGISTER_T reg, uint8_t data, bool isVolatile = false) {
        RESUME();

        RetType ret = CALL(toggleWrite(isVolatile ? WRITE_SET_ENABLE_VOLATILE : WRITE_SET_ENABLE));
        RET_CHECK(ret)
        ret = CALL(m_cs.set(0));
        RET_CHECK(ret)

        ret = CALL(m_spi.write(&data, 1));
        RET_CHECK(ret)

        ret = CALL(m_cs.set(1));
        RET_CHECK(ret)

        RESET();
        return ret;
    }

    RetType write_enable() {
        RESUME();

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        tx_buff[0] = WRITE_SET_ENABLE;

        ret = CALL(m_spi.write(tx_buff, 1));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    RetType write_disable() {
        RESUME();

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        tx_buff[0] = WRITE_SET_DISABLE;

        ret = CALL(m_spi.write(tx_buff, 1));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    RetType write_byte(uint8_t byte, size_t write_address) {
        RESUME();

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        tx_buff[0] = PAGE_PROGRAM;
        tx_buff[1] = (write_address & 0xFF0000) >> 16;
        tx_buff[2] = (write_address & 0x00FF00) >> 8;
        tx_buff[3] = (write_address & 0x0000FF);
        tx_buff[4] = byte;

        ret = CALL(m_spi.write(tx_buff, 5));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    RetType write_page(uint8_t *buff, size_t len, uint32_t page_address, uint32_t byte_offset) {
        RESUME();

        if (len == 0) { // Nothing to do
            RESET();
            return RET_SUCCESS;
        }

        RetType ret = CALL(write_enable());
        RET_CHECK(ret);

        if ((len + byte_offset) > page_size) len = page_size - byte_offset;

        page_address = (page_address * page_size) + byte_offset;



        tx_buff[0] = PAGE_PROGRAM;
        tx_buff[1] = (page_address & 0xFF0000) >> 16;
        tx_buff[2] = (page_address & 0x00FF00) >> 8;
        tx_buff[3] = (page_address & 0x0000FF);

        ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        ret = CALL(m_spi.write(tx_buff, 4));
        RET_CHECK(ret);

        ret = CALL(m_spi.write(buff, len));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));

        RESET();
        return RET_SUCCESS;
    }

    RetType write_sector(uint8_t *buff, size_t len, uint32_t sector_address, uint32_t byte_offset) {
        RESUME();
        static uint32_t start_page;
        static uint32_t bytes_to_write;
        static uint32_t local_offset;

        if (len == 0) { // Nothing to do
            RESET();
            return RET_SUCCESS;
        }

        if (len > sector_size) len = sector_size;
        if (byte_offset > sector_size) RET_CHECK(RET_ERROR);

        if ((byte_offset + len) > sector_size) {
            bytes_to_write = sector_size - byte_offset;
        } else {
            bytes_to_write = len;
        }

        start_page = sectorToPage(sector_address) + (byte_offset / page_size);
        local_offset = byte_offset % page_size;

        do {
            RetType ret = CALL(write_page(buff, bytes_to_write, start_page, local_offset));
            RET_CHECK(ret);

            start_page++;
            bytes_to_write -= page_size - local_offset;
            buff += page_size - local_offset;
            local_offset = 0;
        } while (bytes_to_write > 0);

        RESET();
        return RET_SUCCESS;
    }

    RetType write_block(uint8_t *buff, size_t len, uint32_t block_address, uint32_t byte_offset) {
        RESUME();
        static uint32_t start_page;
        static uint32_t bytes_to_write;
        static uint32_t local_offset;

        if (len == 0) {
            RESET();
            return RET_SUCCESS;
        }
        if (byte_offset > block_size) RET_CHECK(RET_ERROR);
        if (len > block_size) len = block_size;

        if ((byte_offset + len) > block_size) {
            bytes_to_write = block_size - byte_offset;
        } else {
            bytes_to_write = len;
        }

        start_page = blockToPage(block_address);
        local_offset = byte_offset % page_size;
        do {
            RetType ret = CALL(write_page(buff, bytes_to_write, start_page, local_offset));
            RET_CHECK(ret);
            bytes_to_write -= page_size - local_offset;
            buff += page_size - local_offset;
            local_offset = 0;
        } while (bytes_to_write > 0);

        RESET();
        return RET_SUCCESS;
    }

    RetType read_byte(uint8_t *byte, size_t read_address) {
        RESUME();

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        tx_buff[0] = FAST_READ_DUAL_IO;
        tx_buff[1] = (read_address & 0xFF0000) >> 16;
        tx_buff[2] = (read_address & 0x00FF00) >> 8;
        tx_buff[3] = (read_address & 0x0000FF);
        tx_buff[4] = 0;

        // Can prob consolidate into one call
        ret = CALL(m_spi.write(tx_buff, 5));
        RET_CHECK(ret);

        *byte = DUMMY_BYTE;
        ret = CALL(m_spi.write_read(byte, byte, 1));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RESET();
        return ret;
    }

    RetType read_bytes(uint8_t *buff, size_t len, uint32_t read_address) {
        RESUME();

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        tx_buff[0] = FAST_READ_DUAL_IO;
        tx_buff[1] = (read_address & 0xFF0000) >> 16;
        tx_buff[2] = (read_address & 0x00FF00) >> 8;
        tx_buff[3] = (read_address & 0x0000FF);
        tx_buff[4] = 0;

        ret = CALL(m_spi.write(tx_buff, 5));
        RET_CHECK(ret);

        ret = CALL(m_spi.read(buff, len, 2000));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RESET();
        return ret;
    }

    RetType read_page(uint8_t *buff, size_t len, uint32_t page_address, uint32_t byte_offset) {
        RESUME();

        if (len == 0) { // Nothing to do
            RESET();
            return RET_SUCCESS;
        }

        if ((len + byte_offset) > page_size) len = page_size - byte_offset;

        page_address = (page_address * page_size) + byte_offset;

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        tx_buff[0] = FAST_READ_DUAL_IO;
        tx_buff[1] = (page_address & 0xFF0000) >> 16;
        tx_buff[2] = (page_address & 0x00FF00) >> 8;
        tx_buff[3] = (page_address & 0x0000FF);
        tx_buff[4] = 0;

        ret = CALL(m_spi.write(tx_buff, 5));
        RET_CHECK(ret);

        ret = CALL(m_spi.read(buff, len, 100));
        RET_CHECK(ret);


        ret = CALL(m_cs.set(1));
        RESET();
        return RET_SUCCESS;
    }

    RetType read_sector(uint8_t *buff, size_t len, uint32_t sector_address, uint32_t byte_offset) {
        RESUME();
        static uint32_t start_page;
        static uint32_t bytes_to_read;
        static uint32_t local_offset;

        if (len == 0) { // Nothing to do
            RESET();
            return RET_SUCCESS;
        }

        if (len > sector_size) len = sector_size;
        if (byte_offset > sector_size) RET_CHECK(RET_ERROR);

        if ((byte_offset + len) > sector_size) {
            bytes_to_read = sector_size - byte_offset;
        } else {
            bytes_to_read = len;
        }

        start_page = sectorToPage(sector_address) + (byte_offset / page_size);
        local_offset = byte_offset % page_size;

        do {
            RetType ret = CALL(read_page(buff, bytes_to_read, start_page, local_offset)); // TODO: Update
            RET_CHECK(ret);

            start_page++;
            bytes_to_read -= page_size - local_offset;
            buff += page_size - local_offset;
            local_offset = 0;
        } while (bytes_to_read > 0);

        RESET();
        return RET_SUCCESS;
    }

    RetType read_block(uint8_t *buff, size_t len, uint32_t block_address, uint32_t byte_offset) {
        RESUME();
        static uint32_t start_page;
        static uint32_t bytes_to_read;
        static uint32_t local_offset;

        if (len == 0) {
            RESET();
            return RET_SUCCESS;
        }
        if (byte_offset > block_size) RET_CHECK(RET_ERROR);
        if (len > block_size) len = block_size;

        if ((byte_offset + len) > block_size) {
            bytes_to_read = block_size - byte_offset;
        } else {
            bytes_to_read = len;
        }

        start_page = blockToPage(block_address);
        local_offset = byte_offset % page_size;
        do {
            RetType ret = CALL(read_page(buff, bytes_to_read, start_page, local_offset));
            RET_CHECK(ret);
            bytes_to_read -= page_size - local_offset;
            buff += page_size - local_offset;
            local_offset = 0;
        } while (bytes_to_read > 0);

        RESET();
        return RET_SUCCESS;
    }


    RetType erase_chip() {
        RESUME();

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        ret = CALL(m_spi.write(reinterpret_cast<uint8_t *>(0xC7), 1));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RESET();
        return ret;
    }

    RetType erase_sector(uint32_t sector_address) {
        RESUME();
        // TODO: Locking

        sector_address *= sector_size;
        tx_buff[0] = SECTOR_ERASE;
        tx_buff[1] = (sector_address & 0xFF0000) >> 16;
        tx_buff[2] = (sector_address & 0x00FF00) >> 8;
        tx_buff[3] = (sector_address & 0x0000FF);

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        ret = CALL(m_spi.write(tx_buff, 4));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RET_CHECK(ret);

        RESET();
        return ret;
    }

    RetType erase_block(uint32_t block_address) {
        RESUME();

        block_address *= sector_size * 16;
        tx_buff[0] = BLOCK_64_ERASE;
        tx_buff[1] = (block_address & 0xFF0000) >> 16;
        tx_buff[2] = (block_address & 0x00FF00) >> 8;
        tx_buff[3] = (block_address & 0x0000FF);

        RetType ret = CALL(m_cs.set(0));
        RET_CHECK(ret);

        ret = CALL(m_spi.write(tx_buff, 4));
        RET_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RET_CHECK(ret);

        RESET();
        return ret;
    }
};

#endif //LAUNCH_CORE_W25Q_H
