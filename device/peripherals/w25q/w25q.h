/**
 * Platform Independent Implementation for a W25Q Flash Memory Chip
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_W25Q_H
#define LAUNCH_CORE_W25Q_H

#include "device/I2CDevice.h"
#include "device/SPIDevice.h"
#include "device/GPIODevice.h"

#define WQ25Q_CMD(command) { \
    chipSelectPin.set(0); \
    command; \
    chipSelectPin.set(1); \
}

// TODO: Might be a better and the actually correct way of doing 32 bit addresses?
#define ADDR_CMD(address) { \
    dataInPin.set((address & 0xFF000000) >> 16); \
    dataInPin.set((address & 0xFF00) >> 8); \
    dataInPin.set((address & 0xFF)); \
}

class W25Q {
public:
    const uint8_t DUMMY_BYTE = 0xA5;

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


    W25Q(SPIDevice &spiDevice, GPIODevice &csPin, GPIODevice &clkPin, GPIODevice &diPin, GPIODevice &dOutPin) :
            spiDevice(spiDevice), chipSelectPin(csPin), clockPin(clkPin), dataInPin(diPin), dataOutPin(dOutPin) {}

    RetType toggleWrite(WRITE_SET_T command) {
        chipSelectPin.set(0);
        dataInPin.set(command);
        chipSelectPin.set(1);

        return RET_SUCCESS;
    }

    RetType readRegister(READ_STATUS_REGISTER_T reg, uint32_t *data) {
        chipSelectPin.set(0);
        dataInPin.set(reg);
        chipSelectPin.set(1);

        dataOutPin.get(data);

        return RET_SUCCESS;
    }

    RetType writeRegister(WRITE_STATUS_REGISTER_T reg, uint8_t data, bool isVolatile = false) {
        toggleWrite(isVolatile ? WRITE_SET_ENABLE_VOLATILE : WRITE_SET_ENABLE);

        chipSelectPin.set(0);
        dataInPin.set(data);
        chipSelectPin.set(1);

        return RET_SUCCESS;
    }


    RetType readData(READ_COMMAND_T readCommand, uint32_t address, uint8_t *buff, uint32_t *receivedData) {
        // TODO: Validate this. Address is a 24 bit
        uint8_t addrOne = address >> 24;
        uint8_t addrTwo = address >> 16;
        uint8_t addrThree = address >> 8;

        chipSelectPin.set(0);
        uint8_t buffSize = 5;


        switch (readCommand) {
            case READ_DATA: {
                buffSize = 6;
                uint8_t buffArr[] = {readCommand, addrOne, addrTwo, addrThree};

                buff = buffArr;
                break;
            }

            case FAST_READ: {
                buffSize = 14;
                uint8_t buffArr[] = {readCommand, addrOne, addrTwo, addrThree,
                                     DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE,
                                     DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE};

                buff = buffArr;
                break;
            }
            // TODO: Add more
            default: {
                return RET_ERROR;
            }
        }


        RetType ret = dataOutPin.get(receivedData);

        chipSelectPin.set(1);

        return ret;
    }

    RetType writeData(PROGRAM_COMMAND_T programCommand, uint32_t address, uint8_t *page, uint8_t pageSize) {
        // TODO: Maybe return an error if writing is disabled. Should probably do the same for others
        uint8_t addrOne = address >> 24;
        uint8_t addrTwo = address >> 16;
        uint8_t addrThree = address >> 8;
        uint8_t buff[4] = {programCommand, addrOne, addrTwo, addrThree};

        chipSelectPin.set(0);
        dataInPin.set(programCommand);
        dataInPin.set(addrOne);
        dataInPin.set(addrTwo);
        dataInPin.set(addrThree);

        uint8_t i = 0;

        for (int i = 0; i < 256; i++) {
            dataInPin.set(page[i]);
        }

        dataInPin.set(0);


        chipSelectPin.set(1);

        return RET_SUCCESS;
    }

    RetType eraseData(ERASE_COMMAND_T eraseCommand, uint32_t address) {
        toggleWrite(WRITE_SET_ENABLE);

        // TODO: Figure out this 24 bit address thing
        uint8_t addrOne = address >> 24;
        uint8_t addrTwo = address >> 16;
        uint8_t addrThree = address >> 8;
        uint8_t buff[4] = {eraseCommand, addrOne, addrTwo, addrThree};

        chipSelectPin.set(0);
        dataInPin.set(eraseCommand);
        dataInPin.set(addrOne);
        dataInPin.set(addrTwo);
        dataInPin.set(addrThree);
        chipSelectPin.set(1);

        return RET_SUCCESS;
    }
private:
    // TODO: Just realized a SPI device might not be needed if we just have all the pins below :P
    SPIDevice &spiDevice;
    GPIODevice &chipSelectPin;
    GPIODevice &clockPin;
    GPIODevice &dataOutPin;
    GPIODevice &dataInPin;
};

#endif //LAUNCH_CORE_W25Q_H
