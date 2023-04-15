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
#define ADXL375_XYZ_READ_SCALE_FACTOR 49
#define ADXL375_MG2G_MULTIPLIER 0.049
#define ADXL375_GRAVITY 9.80665F
#define DEFAULT_ADXL375(X) ADXL375_Readings X = {.id = 12053, .x_axis = NULL, .y_axis = NULL, .z_axis = NULL}

#include <stdlib.h>
#include <stdint.h>

#include "utils/conversion.h"
#include "sched/macros.h"
#include "return.h"
#include "device/I2CDevice.h"
#include "device/peripherals/SensorDevice.h"


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

typedef struct
{
    uint16_t id;
    uint32_t x_axis;
    uint32_t y_axis;
    uint32_t z_axis;
} ADXL375_Readings

class ADXL375 : public SensorEncodeDecode{
public:
    ADXL375(I2CDevice &i2c) : m_i2c(i2c) {}

    RetType init() {
        RESUME();

        static uint8_t id = 0;

        RetType ret = CALL(m_i2c.read(i2cAddr, &id, 1, 50));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
        if (id != 0xE5) return RET_ERROR;

        ret = CALL(setDataRateAndLowPower(ADXL375_DR_100HZ, false));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(setOperatingMode(ADXL375_MEASURING_MODE));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        SLEEP(100);

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
        i2cAddr.mem_addr = xLSBDataReg;

        RetType ret = CALL(m_i2c.read(i2cAddr, buff, 6));
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
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        i2cAddr.mem_addr = yMSBDataReg;
        ret = CALL(m_i2c.read(i2cAddr, &msb, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

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
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        i2cAddr.mem_addr = offsetYReg;
        ret = CALL(m_i2c.write(i2cAddr, yOffBuff, 2));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        i2cAddr.mem_addr = offsetZReg;
        ret = CALL(m_i2c.write(i2cAddr, zOffBuff, 2));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }


        RESET();
        return ret;
    }

    void encode(void* sensor_struct, uint8_t buffer){
        ADXL375_Readings data = (ADXL375_Readings)sensor_struct;
        uint16_to_uint8(data->id, buffer);
        uint32_to_uint8(data->x_axis, buffer + 2);
        uint32_to_uint8(data->y_axis, buffer + 6);
        uint32_to_uint8(data->z_axis, buffer + 10);
    }

    void decode(void* sensor_struct, uint8_t buffer){
        ADXL375_Readings data = (ADXL375_Readings)sensor_struct;
        data->id = uint8_to_int16(buffer);
        data->x_axis = uint8_to_uint32(buffer + 2);
        data->y_axis = uint8_to_uint32(buffer + 6);
        data->z_axis = uint8_to_uint32(buffer + 10);
    }

private:
    I2CDevice &m_i2c;
    I2CAddr_t i2cAddr{
            .dev_addr = ADXL375_DEV_ADDR_SEC << 1,
            .mem_addr = 0x00,
            .mem_addr_size = 1
    };
};


#endif //LAUNCH_CORE_ADXL375_H
