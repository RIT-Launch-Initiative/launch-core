/**
 * ADXL375 Accelerometer Driver
 *
 * @author Aaron Chan and Akhil D
 */

#ifndef LAUNCH_CORE_ADXL375_H
#define LAUNCH_CORE_ADXL375_H

#define ADXL375_DEV_ADDR_PRIM 0x3B
#define ADXL375_DEV_ADDR_SEC 0x53
#define ADXL375_REG_BW_RATE 0x2C


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

// TODO: Define possible configurations (i.e data rates)
typedef enum {
    ADXL375_DR_3200HZ = 0x0F,
    ADXL375_DR_1600HZ = 0x0E,
    ADXL375_DR_800HZ = 0x0D,
    ADXL375_DR_400HZ = 0x0C,
    ADXL375_DR_200HZ = 0x0B,
    ADXL375_DR_100HZ = 0x0A,
    ADXL375_DR_50HZ = 0x09,
    ADXL375_DR_25HZ = 0x08,
    ADXL375_DR_12HZ5 = 0x07,
    ADXL375_DR_6HZ25 = 0x06,
} ADXL375_DATA_RATE;

typedef enum{
    ADXL375_MEASURING_MODE = 0x08,
    ADLX375_SLEEP_MODE = 0x04,
    ADLX375_AUTOSLEEP_MODE = 0x10,
} ADXL375_OP_MODE;

class ADXL375 {
public:
    ADXL375(I2CDevice &i2c) : m_i2c(i2c) {}

    RetType init() {
        RESUME();

        static uint8_t id = 0;

        RetType ret = CALL(m_i2c.read(i2cAddr, &id, 1));
        if (ret != RET_SUCCESS) return ret;
        if (id != 0xE5) return RET_ERROR;

        // TODO: Call the 2 new functions

        RESET();
        return RET_SUCCESS;
    }

    RetType readXYZ(int16_t *xAxis, int16_t *yAxis, int16_t *zAxis) {
        RESUME();

        RetType ret = CALL(readX(xAxis));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(readY(yAxis));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(readZ(zAxis));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType readX(int16_t *xAxis) {
        RESUME();
        static uint8_t lsb = 0;
        static uint8_t msb = 0;

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
        static uint8_t lsb = 0;
        static uint8_t msb = 0;

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
        static uint8_t lsb = 0;
        static uint8_t msb = 0;

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

    RetType setDataRate(ADXL375_DATA_RATE dataRate) {
        RESUME();
        i2cAddr.mem_addr = ADXL375_REG_BW_RATE;
        RetType ret = CALL(m_i2c.write(i2cAddr, &dataRate, 1));
        RESET();
        return ret;
    }

    // TODO: Set Operating Mode Function
    RetType setOperatingMode(ADXL375_OP_MODE opMode){
        RESUME();
        i2cAddr.mem_addr = ADXL375_REG_BW_RATE;
        RetType ret = CALL(m_i2c.write(i2cAddr,  &opMode, 1));
        RESET();
        return ret;
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
