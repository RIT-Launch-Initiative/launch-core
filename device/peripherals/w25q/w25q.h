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

    W25Q(SPIDevice &spiDevice, GPIODevice &csPin, GPIODevice &clkPin) :
            BlockDevice("FlashDevice"), spiDevice(spiDevice), chipSelectPin(csPin), clockPin(clkPin) {}


    RetType init() {
        RESUME();
        uint32_t deviceID = 0;

        RetType ret = CALL(readID(&deviceID));
        if (ret != RET_SUCCESS) return ret;

        if (deviceID != 0x17) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType toggleWrite(WRITE_SET_T command) {
        RESUME();
        RetType ret = CALL(chipSelectPin.set(0));
        RET_CHECK(ret)

        ret = CALL(spiDevice.write(reinterpret_cast<uint8_t *>(&command), 1));
        RET_CHECK(ret)

        ret = CALL(chipSelectPin.set(1));
        RET_CHECK(ret)

        RESET();
        return ret;
    }

    RetType write(size_t block, uint8_t* buff) override {
        RESUME();

        RetType ret = CALL(writeData(PAGE_PROGRAM, block, buff, getBlockSize()));
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    RetType read(size_t block, uint8_t* buff) override {
        RESUME();

        uint8_t commandBuff[4];
        RetType ret = CALL(readData(READ_DATA, block, commandBuff, buff, getBlockSize()));
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    RetType readRegister(READ_STATUS_REGISTER_T reg, uint8_t *receiveBuff) {
        RESUME();

        RetType ret = CALL(chipSelectPin.set(0));
        RET_CHECK(ret)

        ret = spiDevice.write(reinterpret_cast<uint8_t *>(&reg), 1);
        RET_CHECK(ret)

        ret = chipSelectPin.set(1);
        RET_CHECK(ret)

        // TODO: Figure out possibility chipSelectPin memory to be borked
        ret = CALL(spiDevice.read(receiveBuff, 1));
        RET_CHECK(ret)

        RESET();
        return ret;
    }

    RetType writeRegister(WRITE_STATUS_REGISTER_T reg, uint8_t data, bool isVolatile = false) {
        RESUME();

        RetType ret = CALL(toggleWrite(isVolatile ? WRITE_SET_ENABLE_VOLATILE : WRITE_SET_ENABLE));
        RET_CHECK(ret)
        ret = CALL(chipSelectPin.set(0));
        RET_CHECK(ret)

        ret = CALL(spiDevice.write(&data, 1));
        RET_CHECK(ret)

        ret = CALL(chipSelectPin.set(1));
        RET_CHECK(ret)

        RESET();
        return ret;
    }


    RetType readData(READ_COMMAND_T readCommand, uint32_t address, uint8_t *buff, uint8_t *receivedData, size_t receivedSize) {
        RESUME();

        RetType ret = CALL(chipSelectPin.set(0));
        RET_CHECK(ret);

        uint8_t buffSize = 6;

        switch (readCommand) {
            case READ_DATA: {
                buffSize = 4;
                uint8_t buffArr[] = {static_cast<uint8_t>(readCommand),
                                     static_cast<uint8_t>((address & 0xFF000000) >> 16),
                                     static_cast<uint8_t>((address & 0xFF000000) >> 8),
                                     static_cast<uint8_t>((address & 0xFF000000))};

                buff = buffArr;
                break;
            }

            case FAST_READ: {
                buffSize = 14;
                uint8_t buffArr[] = {static_cast<uint8_t>(readCommand),
                                     static_cast<uint8_t>((address & 0xFF000000) >> 16),
                                     static_cast<uint8_t>((address & 0xFF000000) >> 8),
                                     static_cast<uint8_t>((address & 0xFF000000)),
                                     DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE,
                                     DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE};

                buff = buffArr;
                break;
            }
            default: {
                RESET();
                return RET_ERROR;
            }
        }

        ret = CALL(spiDevice.write(buff, buffSize));
        RET_CHECK(ret)

        ret = CALL(spiDevice.read(receivedData, receivedSize));
        RET_CHECK(ret)

        ret = CALL(chipSelectPin.set(1));
        RET_CHECK(ret)

        RESET();
        return ret;
    }

    // Make sure end of page is 0
    RetType writeData(PROGRAM_COMMAND_T programCommand, uint32_t address, uint8_t *page, size_t pageSize) {
        RESUME();

        // TODO: Maybe return an error if writing is disabled. Should probably do the same for others
        static uint8_t *buff;
        RetType ret = chipSelectPin.set(0);
        RET_CHECK(ret);

        uint8_t buffer[5] = {static_cast<uint8_t>(programCommand),
                             static_cast<uint8_t>((address & 0xFF000000) >> 16),
                             static_cast<uint8_t>((address & 0xFF000000) >> 8),
                             static_cast<uint8_t>((address & 0xFF000000))};

        buff = buffer;

        ret = CALL(spiDevice.write(buff, 5));
        RET_CHECK(ret)

        ret = CALL(spiDevice.write(page, pageSize));
        RET_CHECK(ret)

        ret = chipSelectPin.set(1);
        RET_CHECK(ret)

        RESET();
        return RET_SUCCESS;
    }

    RetType eraseData(ERASE_COMMAND_T eraseCommand, uint32_t address) {
        RESUME();

        this->toggleWrite(WRITE_SET_ENABLE);

        uint8_t buff[5] = {static_cast<uint8_t>(eraseCommand),
                           static_cast<uint8_t>((address & 0xFF000000) >> 16),
                           static_cast<uint8_t>((address & 0xFF000000) >> 8),
                           static_cast<uint8_t>((address & 0xFF000000))};

        RetType ret = CALL(chipSelectPin.set(0));
        RET_CHECK(ret);

        ret = CALL(spiDevice.write(buff, 5));
        RET_CHECK(ret);

        ret = CALL(chipSelectPin.set(1));
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }



    size_t getBlockSize() override {
        return 256;
    }

    size_t getNumBlocks() override {
        return 65536;
    }

    // TODO: Should these need to be implemented?
    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        return RET_SUCCESS;
    }

private:
    SPIDevice &spiDevice;
    GPIODevice &chipSelectPin;
    GPIODevice &clockPin;

    RetType readID(uint32_t *deviceID) {
        RESUME();

        uint8_t buff[4] = {0x9F, 0x00, 0x00, 0x00};

        RetType ret = CALL(chipSelectPin.set(0));
        RET_CHECK(ret);

        ret = spiDevice.write(buff, 4);
        RET_CHECK(ret)

        ret = spiDevice.read(buff, 4);
        RET_CHECK(ret)

        ret = chipSelectPin.set(1);
        RET_CHECK(ret);

        *deviceID = (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | buff[3];

        RESET();
        return RET_SUCCESS;
    }
};

#endif //LAUNCH_CORE_W25Q_H
