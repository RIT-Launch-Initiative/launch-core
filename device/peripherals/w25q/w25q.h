/**
 * Platform Independent Implementation for a W25Q Flash Memory Chip
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_W25Q_H
#define LAUNCH_CORE_W25Q_H

#include "device/I2CDevice.h"
#include "device/SPIDevice.h"


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

    W25Q(SPIDevice& spiDevice) : spiDevice(spiDevice) {}

    RetType read() {
        const uint8_t READ_COMMAND = 0x03;


        return RET_SUCCESS;
    }


    RetType readData(READ_COMMAND_T readCommand, uint32_t address) {
        uint8_t addrOne = address >> 24;
        uint8_t addrTwo = address >> 16;
        uint8_t addrThree = address >> 8;
        uint8_t addrFour = address;

        uint8_t buff[6] = {readCommand, addrOne, addrTwo, addrThree, addrFour};

        spiDevice.read(buff, sizeof(buff));


        return RET_SUCCESS;
    }

    RetType write() {
        return RET_SUCCESS;
    }

private:
    SPIDevice& spiDevice;

};

#endif //LAUNCH_CORE_W25Q_H
