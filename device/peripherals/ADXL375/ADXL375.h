/**
 * ADXL375 Accelerometer Driver
 *
 * @author Aaron Chan and Akhil D
 */

#ifndef LAUNCH_CORE_ADXL375_H
#define LAUNCH_CORE_ADXL375_H

#define ADXL375_DEV_ADDR_PRIM 0x3B
#define ADXL375_DEV_ADDR_SEC 0x53


#include <stdlib.h>
#include <stdint.h>

#include "sched/macros.h"
#include "return.h"
#include "device/I2CDevice.h"


typedef enum {
    xLSBDataReg = 0x32,
    xMSBDataReg = 0x33,
    yLSBDataReg = 0x34,
    yMSBDataReg = 0x35,
    zLSBDataReg = 0x36,
    zMSBDataReg = 0x37,

    deviceID = 0x00,
} ADXL375_REG;

class ADXL375 {
public:
    ADXL375(I2CDevice &i2c) : m_i2c(i2c) {}

    RetType init() {
        RESUME();

        static uint8_t id = 0;

        RetType ret = CALL(m_i2c.read(i2cAddr, &id, 1));
        if (ret != RET_SUCCESS) return ret;
        if (id != 0xE5) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType readXYZ(int16_t *xAxis, int16_t *yAxis, int16_t *zAxis) {
        RESUME();

        RetType ret = readX(xAxis);
        if (ret != RET_SUCCESS) return ret;

        ret = readY(yAxis);
        if (ret != RET_SUCCESS) return ret;

        ret = readY(yAxis);
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType readX(int16_t *xAxis) {
        RESUME();
        uint8_t lsb = 0;
        uint8_t msb = 0;

        // reading the data
        i2cAddr.mem_addr = xLSBDataReg;
        RetType ret = CALL(m_i2c.read(i2cAddr, &lsb, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        i2cAddr.mem_addr = xMSBDataReg;
        ret = CALL(m_i2c.read(i2cAddr, &msb, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // value is in 2's complement so have to convert it
        *xAxis = ((msb << 8) | lsb) * -1;

        RESET();
        return RET_SUCCESS;
    }

    RetType readY(int16_t *yAxis) {
        RESUME();
        uint8_t lsb = 0;
        uint8_t msb = 0;

        // reading the data
        i2cAddr.mem_addr = yLSBDataReg;
        RetType ret = CALL(m_i2c.read(i2cAddr, &lsb, 1));
        if (ret != RET_SUCCESS) return ret;

        i2cAddr.mem_addr = yMSBDataReg;
        ret = CALL(m_i2c.read(i2cAddr, &msb, 1));
        if (ret != RET_SUCCESS) return ret;

        // value is in 2's complement so have to convert it
        *yAxis = ((msb << 8) | lsb) * -1;

        RESET();
        return RET_SUCCESS;
    }

    RetType readZ(int16_t *zAxis) {
        RESUME();
        uint8_t lsb = 0;
        uint8_t msb = 0;

        // reading the data
        i2cAddr.mem_addr = zLSBDataReg;
        RetType ret = CALL(m_i2c.read(i2cAddr, &lsb, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        i2cAddr.mem_addr = zMSBDataReg;
        ret = CALL(m_i2c.read(i2cAddr, &msb, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        *zAxis = ((msb << 8) | lsb) * -1;

        RESET();
        return RET_SUCCESS;

    }


private:
    I2CDevice &m_i2c;
    I2CAddr_t i2cAddr {
        .dev_addr = ADXL375_DEV_ADDR_SEC << 1,
        .mem_addr = 0x00,
        .mem_addr_size = 1
    };
};


#endif //LAUNCH_CORE_ADXL375_H
