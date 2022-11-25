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


class W25Q {
public:
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

    } WRITE_COMMAND_T;

    typedef enum {
        WRITE_SET_ENABLE = 0x06,
        WRITE_SET_ENABLE_VOLATILE = 0x50,
        WRITE_SET_DISABLE = 0x04,
    } WRITE_SET_T;

    typedef enum {
        REGISTER_ONE_STATUS = 0x05;
        REGISTER_TWO_STATUS = 0x35;
        REGISTER_THREE_STATUS = 0x15;

    } READ_STATUS_REGISTER_T;


    W25Q(SPIDevice &spiDevice, GPIODevice &csPin, GPIODevice &clkPin, GPIODevice &diPin, GPIODevice &dOutPin) :
            spiDevice(spiDevice), chipSelectPin(csPin), clockPin(clkPin), dataInPin(diPin), dataOutPin(dOutPin) {}

    RetType read(uint8_t *buff, size_t len) {
        return spiDevice.read(buff, len);
    }

    RetType toggleWrite(bool toggled) {
        uint8_t command = toggled ? WRITE_SET_ENABLE : WRITE_SET_DISABLE;

        chipSelectPin.set(0);
        dataInPin.set(command);
        chipSelectPin.set(1);

        return RET_SUCCESS;
    }

    RetType readRegister(READ_STATUS_REGISTER_T reg) {
        chipSelectPin.set(0);
        dataInPin.set(reg);
        chipSelectPin.set(1);

        return RET_SUCCESS;
    }


    RetType readData(READ_COMMAND_T readCommand, uint32_t address) {
        uint8_t addrOne = address >> 24;
        uint8_t addrTwo = address >> 16;
        uint8_t addrThree = address >> 8;
        uint8_t addrFour = address;

        chipSelectPin.set(0);

        uint8_t buff[6] = {readCommand, addrOne, addrTwo, addrThree, addrFour};

        spiDevice.read(buff, sizeof(buff));


        return RET_SUCCESS;
    }

    RetType write(WRITE_COMMAND_T writeCommand, uint32_t address) {
        return RET_SUCCESS;
    }

private:
    SPIDevice &spiDevice;
    GPIODevice &chipSelectPin;
    GPIODevice &clockPin;
    GPIODevice &dataOutPin;
    GPIODevice &dataInPin;
};

#endif //LAUNCH_CORE_W25Q_H
