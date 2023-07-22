/**
 * ADXL375 Accelerometer Driver
 *
 * @author Aaron Chan
 * @author Akhil D
 */

#ifndef LAUNCH_CORE_ADXL375_H
#define LAUNCH_CORE_ADXL375_H

#define ADXL375_DEV_ADDR_PRIM 0x3B
#define ADXL375_DEV_ADDR_SEC 0x53
#define ADXL375_REG_BW_RATE 0x2C
#define ADXL375_POWER_CTL 0x2D
#define ADXL375_REG_DATA_FORMAT 0x31
#define ADXL375_XYZ_READ_SCALE_FACTOR 49
#define ADXL375_MG2G_MULTIPLIER 0.049
#define ADXL375_GRAVITY 9.80665F

#define ADXL375_DATA_STRUCT(variable_name) ADXL375_DATA_T variable_name = {.id = 12000, .x_accel = 0, .y_accel = 0, .z_accel = 0}

#include <stdlib.h>
#include <stdint.h>

#include "utils/conversion.h"
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
    ADXL375_SLEEP_MODE = 0x04,
    ADXL375_AUTOSLEEP_MODE = 0x10,
} ADXL375_OP_MODE;

using ADXL375_DATA_T = struct {
    const uint16_t id;
    int16_t x_accel;
    int16_t y_accel;
    int16_t z_accel;
};

class ADXL375 : public Device {
public:
    explicit ADXL375(I2CDevice &i2c, const uint16_t address = ADXL375_DEV_ADDR_PRIM, const char* name = "ADXl375") : Device(name), m_i2c(&i2c),
                                                                                         i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

    RetType init() override {
        RESUME();

        static uint8_t id = 0;
        RetType ret = CALL(readID(&id));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        if (id != 0xE5) {
            RESET();
            return RET_ERROR;
        }

//        ret = CALL(setOperatingMode(ADXL375_SLEEP_MODE)); // TODO: Might not be able to set settings in sleep mode
//        if (ret != RET_SUCCESS) {and Akhil D
//            RESET();
//            return ret;
//        }

        ret = CALL(setDataRateAndLowPower(ADXL375_DR_100HZ, false));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(setRange(0b00001011));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(setOperatingMode(ADXL375_MEASURING_MODE));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(wakeup());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType getData(ADXL375_DATA_T *data) {
        RESUME();

        RetType ret = CALL(readXYZ(&data->x_accel, &data->y_accel, &data->z_accel));

        RESET();
        return RET_SUCCESS;
    }

    RetType readXYZ(int16_t *xAxis, int16_t *yAxis, int16_t *zAxis) {
        RESUME();

        RetType ret = CALL(readXYZRaw(xAxis, yAxis, zAxis));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        *xAxis *= ADXL375_MG2G_MULTIPLIER * ADXL375_GRAVITY;
        *yAxis *= ADXL375_MG2G_MULTIPLIER * ADXL375_GRAVITY;
        *zAxis *= ADXL375_MG2G_MULTIPLIER * ADXL375_GRAVITY;

        RESET();
        return RET_SUCCESS;
    }

    RetType readXYZFloat(float *xAxis, float *yAxis, float *zAxis) {
        RESUME();

        static int16_t xRaw;
        static int16_t yRaw;
        static int16_t zRaw;

        RetType ret = CALL(readXYZRaw(&xRaw, &yRaw, &zRaw));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        *xAxis = xRaw * ADXL375_MG2G_MULTIPLIER * ADXL375_GRAVITY;
        *yAxis = yRaw * ADXL375_MG2G_MULTIPLIER * ADXL375_GRAVITY;
        *zAxis = zRaw * ADXL375_MG2G_MULTIPLIER * ADXL375_GRAVITY;

        RESET();
        return RET_SUCCESS;
    }


    RetType readXYZRaw(int16_t *xAxis, int16_t *yAxis, int16_t *zAxis) {
        RESUME();

        static uint8_t buff[6] = {0};

        RetType ret = CALL(readReg(xLSBDataReg, buff, 6));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        *xAxis = ((buff[1] << 8) | buff[0]) * -1;
        *yAxis = ((buff[3] << 8) | buff[2]) * -1;
        *zAxis = ((buff[5] << 8) | buff[4]) * -1;

        RESET();
        return RET_SUCCESS;
    }

    RetType readX(int16_t *xAxis) {
        RESUME();

        RetType ret = CALL(readAxis(xAxis, xLSBDataReg));

        RESET();
        return RET_SUCCESS;
    }

    RetType readY(int16_t *yAxis) {
        RESUME();

        RetType ret = CALL(readAxis(yAxis, yLSBDataReg));

        RESET();
        return RET_SUCCESS;
    }

    RetType readZ(int16_t *zAxis) {
        RESUME();

        RetType ret = CALL(readAxis(zAxis, zLSBDataReg));

        RESET();
        return ret;
    }

    RetType readAxis(int16_t *axis, ADXL375_REG axisReg) {
        RESUME();
        static uint8_t data[2];

        RetType ret = CALL(readReg(axisReg, data, 2));
        *axis = ((data[1] << 8) | data[2]) * -1;

        RESET();
        return ret;
    }

    RetType wakeup() {
        RESUME();

        static uint8_t val = 0x08;
        RetType ret = CALL(writeReg(ADXL375_POWER_CTL, &val));
        
        RESET();
        return ret;
    }

    RetType setDataRateAndLowPower(ADXL375_DATA_RATE dataRate, bool lowPower) {
        RESUME();
        i2cAddr.mem_addr = ADXL375_REG_BW_RATE;
        static uint8_t rate = static_cast<uint8_t>(dataRate);
        if (lowPower) rate |= 0x8;

        RetType ret = CALL(writeReg(ADXL375_REG_BW_RATE, &rate));
        RESET();
        return ret;
    }

    RetType setOperatingMode(ADXL375_OP_MODE opMode) {
        RESUME();

        static uint8_t operating_mode;
        operating_mode = static_cast<uint8_t>(opMode);

        RetType ret = CALL(writeReg(ADXL375_POWER_CTL, &operating_mode));

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

        RetType ret = CALL(writeReg(offsetXReg, xOffBuff, 2));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
        ret = CALL(writeReg(offsetYReg, yOffBuff, 2));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(offsetZReg, zOffBuff, 2));

        RESET();
        return ret;
    }

    RetType setRange(uint8_t range) {
        RESUME();
        RetType ret = CALL(writeReg(ADXL375_REG_DATA_FORMAT, &range));
        RESET();
        return ret;
    }

private:
    I2CDevice *m_i2c;
    I2CAddr_t i2cAddr;

    RetType readID(uint8_t *id) {
        RESUME();

        RetType ret = CALL(readReg(0x00, id));

        RESET();
        return ret;
    }

    RetType readReg(uint8_t command, uint8_t *value, size_t len = 1) {
        RESUME();

        i2cAddr.mem_addr = command;
        RetType ret = CALL(m_i2c->read(i2cAddr, value, len));

        RESET();
        return ret;
    }

    RetType writeReg(uint8_t command, uint8_t *value, size_t len = 1) {
        RESUME();

        i2cAddr.mem_addr = command;
        RetType ret = CALL(m_i2c->write(i2cAddr, value, len));

        RESET();
        return ret;
    }

};


#endif //LAUNCH_CORE_ADXL375_H
