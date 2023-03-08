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
#define ADXL375_POWER_CTL 0x2D
#define ADXL375_REG_DATA_FORMAT 0x31

#define ADXL375_XYZ_READ_SCALE_FACTOR   49


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

    offsetXReg = 0x1E,
    offsetYReg = 0x1F,
    offsetZReg = 0x20,

    deviceID = 0x00,
} ADXL375_REG;

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

typedef enum {
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

        static int16_t xAvg;
        static int16_t yAvg;
        static int16_t zAvg;


        RetType ret = CALL(m_i2c.read(i2cAddr, &id, 1));
        if (ret != RET_SUCCESS) return ret;
        if (id != 0xE5) return RET_ERROR;

        ret = CALL(setDataRateAndLowPower(ADXL375_DR_100HZ, false));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setOperatingMode(ADXL375_MEASURING_MODE));
        if (ret != RET_SUCCESS) return ret;

        SLEEP(100);


//        for (int i = 0; i < 10; i++) {
//
//            xAvg += x;
//            yAvg += y;
//            zAvg += z;
//        }
//
//        xAvg /= 10;
//        yAvg /= 10;
//        zAvg /= 10;


        RESET();
        return RET_SUCCESS;
    }

    RetType calibrate() {
        RESUME();

        static int16_t x;
        static int16_t y;
        static int16_t z;

        static int16_t xOff;
        static int16_t yOff;
        static int16_t zOff;

        static int16_t xOn;
        static int16_t yOn;
        static int16_t zOn;

        for (int i = 0; i < 10; i++) {
            ret = CALL(readXYZ(&x, &y, &z));
            if (ret != RET_SUCCESS) return ret;

            xOff += x;
            yOff += y;
            zOff += z;
        }

        xOff /= 10;
        yOff /= 10;
        zOff /= 10;

        ret = CALL(selfTest(&xOn, &yOn, &zOn));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setOffset(400, 23, 28));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType readXYZ(int16_t *xAxis, int16_t *yAxis, int16_t *zAxis) {
        RESUME();

        static uint8_t buff[6] = {0};
        i2cAddr.mem_addr = xLSBDataReg;

        RetType ret = CALL(m_i2c.read(i2cAddr, buff, 6));
        if (ret != RET_SUCCESS) return ret;

        *xAxis = ((buff[1] << 8) | buff[0]) * -1;
        *yAxis = ((buff[3] << 8) | buff[2]) * -1;
        *zAxis = ((buff[5] << 8) | buff[4]) * -1;

        RESET();
        return RET_SUCCESS;
    }

    RetType readXYZRaw(int16_t *xAxis, int16_t *yAxis, int16_t *zAxis) {
        RESUME();

        static uint8_t buff[6] = {0};
        i2cAddr.mem_addr = xLSBDataReg;

        RetType ret = CALL(m_i2c.read(i2cAddr, buff, 6));
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

    RetType wakeup() {
        RESUME();

        i2cAddr.mem_addr = ADXL375_POWER_CTL;
        RetType ret = CALL(m_i2c.write(i2cAddr, reinterpret_cast<uint8_t *>(0x08), 1));

        RESET();
        return ret;
    }

    RetType setDataRateAndLowPower(ADXL375_DATA_RATE dataRate, bool lowPower) {
        RESUME();
        i2cAddr.mem_addr = ADXL375_REG_BW_RATE;
        static uint8_t rate = static_cast<uint8_t>(dataRate);
        if (lowPower) rate |= 0x8;

        RetType ret = CALL(m_i2c.write(i2cAddr, &rate, 1));
        RESET();
        return ret;
    }

    RetType setOperatingMode(ADXL375_OP_MODE opMode) {
        RESUME();
        i2cAddr.mem_addr = ADXL375_POWER_CTL;
        RetType ret = CALL(m_i2c.write(i2cAddr, reinterpret_cast<uint8_t *>(&opMode), 1));
        RESET();
        return ret;
    }

    RetType setOffset(int16_t xOffset, int16_t yOffset, int16_t zOffset) {
        RESUME();

        static uint8_t xOffBuff[2] = {static_cast<uint8_t>(xOffset & 0xFF),
                                      static_cast<uint8_t>((xOffset >> 8) & 0xFF)};
        static uint8_t yOffBuff[2] = {static_cast<uint8_t>(yOffset & 0xFF),
                                      static_cast<uint8_t>((yOffset >> 8) & 0xFF)};
        static uint8_t zOffBuff[2] = {static_cast<uint8_t>(zOffset & 0xFF),
                                      static_cast<uint8_t>((zOffset >> 8) & 0xFF)};


        i2cAddr.mem_addr = offsetXReg;
        RetType ret = CALL(m_i2c.write(i2cAddr, xOffBuff, 2));
        if (ret != RET_SUCCESS) return ret;

        i2cAddr.mem_addr = offsetYReg;
        ret = CALL(m_i2c.write(i2cAddr, yOffBuff, 2));
        if (ret != RET_SUCCESS) return ret;

        i2cAddr.mem_addr = offsetZReg;
        ret = CALL(m_i2c.write(i2cAddr, zOffBuff, 2));
        if (ret != RET_SUCCESS) return ret;


        RESET();
        return ret;
    }

private:
    I2CDevice &m_i2c;
    I2CAddr_t i2cAddr{
            .dev_addr = ADXL375_DEV_ADDR_SEC << 1,
            .mem_addr = 0x00,
            .mem_addr_size = 1
    };

    RetType selfTest(int16_t *xAvg, int16_t *yAvg, int16_t *zAvg) {
        RESUME();

        static uint8_t dataFormat = 0;
        static int16_t x = 0;
        static int16_t y = 0;
        static int16_t z = 0;

        i2cAddr.mem_addr = ADXL375_REG_DATA_FORMAT;
        RetType ret = CALL(m_i2c.read(i2cAddr, &dataFormat, 1));
        if (ret != RET_SUCCESS) return ret;

        dataFormat |= (1 << 7);
        ret = CALL(m_i2c.write(i2cAddr, &dataFormat, 1));
        if (ret != RET_SUCCESS) return ret;

        for (int i = 0; i < 10; i++) {
            ret = CALL(readXYZ(&x, &y, &z));
            if (ret != RET_SUCCESS) return ret;

            *xAvg += x;
            *yAvg += y;
            *zAvg += z;
        }

        *xAvg /= 10;
        *yAvg /= 10;
        *zAvg /= 10;

        dataFormat &= ~(1 << 7);
        ret = CALL(m_i2c.write(i2cAddr, &dataFormat, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }


};


#endif //LAUNCH_CORE_ADXL375_H
