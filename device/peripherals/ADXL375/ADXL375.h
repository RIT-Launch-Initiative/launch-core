/**
 * ADXL375 Accelerometer Driver
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_ADXL375_H
#define LAUNCH_CORE_ADXL375_H

#define ADXL375_DATA_STRUCT(variable_name) ADXL375::ADXL375_DATA_T variable_name = {.id = 12000, .x_accel = 0, .y_accel = 0, .z_accel = 0}

#include <stdlib.h>
#include <stdint.h>

#include "utils/conversion.h"
#include "sched/macros.h"
#include "return.h"
#include "device/I2CDevice.h"

class ADXL375 : public Device {
public:
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

    typedef struct {
        const uint16_t id;
        float x_accel;
        float y_accel;
        float z_accel;
    } ADXL375_DATA_T;

    typedef struct {
        uint8_t data_rate;
        uint8_t operating_mode;
        uint8_t range;
    } ADXL375_CONFIG_T;

    static constexpr uint8_t ADXL375_DEV_ADDR_PRIM = 0x3B;
    static constexpr uint8_t ADXL375_DEV_ADDR_SEC = 0x53;
    static constexpr uint8_t ADXL375_REG_BW_RATE = 0x2C;
    static constexpr uint8_t ADXL375_POWER_CTL = 0x2D;
    static constexpr uint8_t ADXL375_REG_DATA_FORMAT = 0x31;
    static constexpr uint8_t ADXL375_XYZ_READ_SCALE_FACTOR = 49;
    static constexpr float ADXL375_MG2G_MULTIPLIER = 0.049;
    static constexpr float ADXL375_GRAVITY = 9.80665F;

    explicit ADXL375(I2CDevice &i2c, const uint16_t address = ADXL375_DEV_ADDR_PRIM, const char *name = "ADXl375")
            : Device(name), m_i2c(&i2c),
              i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

    RetType init() override {
        RESUME();

        RetType ret = CALL(checkID());
        if (RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        ret = CALL(setDataRateAndLowPower(ADXL375_DR_100HZ, false));
        if (RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        ret = CALL(setRange(0b00001011));
        if (RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        ret = CALL(setOperatingMode(ADXL375_MEASURING_MODE));
        if (RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        ret = CALL(wakeup());
        if (RET_SUCCESS != ret) {
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

    RetType readXYZ(float *xAxis, float *yAxis, float *zAxis) {
        constexpr float scale = ADXL375_MG2G_MULTIPLIER * ADXL375_GRAVITY;
        constexpr float bound = 10000;

        RESUME();

        RetType ret = CALL(readReg(xLSBDataReg, m_buff, 6));
        if (RET_SUCCESS != ret) {
            RESET();
            return ret;
        }


        *xAxis = static_cast<int16_t>((m_buff[1] << 8) | m_buff[0]) * scale;
        *yAxis = static_cast<int16_t>((m_buff[3] << 8) | m_buff[2]) * scale;
        *zAxis = static_cast<int16_t>((m_buff[5] << 8) | m_buff[4]) * scale;

        if ((bound < abs(*xAxis)) || (bound < abs(*yAxis)) || (bound < abs(*zAxis)) ) {
            ret = RET_ERROR; 
        }

        RESET();
        return ret;
    }

    RetType readX(int16_t *xAxis) {
        RESUME();

        RetType ret = CALL(readAxis(xAxis, xLSBDataReg));

        RESET();
        return ret;
    }

    RetType readY(int16_t *yAxis) {
        RESUME();

        RetType ret = CALL(readAxis(yAxis, yLSBDataReg));

        RESET();
        return ret;
    }

    RetType readZ(int16_t *zAxis) {
        RESUME();

        RetType ret = CALL(readAxis(zAxis, zLSBDataReg));

        RESET();
        return ret;
    }

    RetType readAxis(int16_t *axis, ADXL375_REG axisReg) {
        RESUME();

        RetType ret = CALL(readReg(axisReg, m_buff, 2));
        *axis = ((m_buff[0] << 8) | m_buff[1]) * -1;

        RESET();
        return ret;
    }

    RetType wakeup() {
        RESUME();

        m_buff[0] = 0x08;
        RetType ret = CALL(writeReg(ADXL375_POWER_CTL, m_buff, 1));

        RESET();
        return ret;
    }

    RetType setDataRateAndLowPower(ADXL375_DATA_RATE dataRate, bool lowPower) {
        RESUME();

        m_buff[0] = dataRate;
        if (lowPower) m_buff[0] |= 0x8;

        RetType ret = CALL(writeReg(ADXL375_REG_BW_RATE, m_buff, 1));

        RESET();
        return ret;
    }

    RetType setOperatingMode(ADXL375_OP_MODE opMode) {
        RESUME();

        m_buff[0] = opMode;
        RetType ret = CALL(writeReg(ADXL375_POWER_CTL, m_buff, 1));

        RESET();
        return ret;
    }

    RetType setOffset(int16_t xOffset, int16_t yOffset, int16_t zOffset) {
        RESUME();

        m_buff[0] = static_cast<uint8_t>(xOffset & 0xFF);
        m_buff[1] = static_cast<uint8_t>((xOffset >> 8) & 0xFF);

        // Y Offset
        m_buff[2] = static_cast<uint8_t>(yOffset & 0xFF);
        m_buff[3] = static_cast<uint8_t>((yOffset >> 8) & 0xFF);

        // Z Offset
        m_buff[4] = static_cast<uint8_t>(zOffset & 0xFF);
        m_buff[5] = static_cast<uint8_t>((zOffset >> 8) & 0xFF);

        i2cAddr.mem_addr_size = 2; // TODO: Figure out if we can just make this one call and address sizing
        RetType ret = CALL(writeReg(offsetXReg, m_buff, 2));
        i2cAddr.mem_addr_size = 1;

//        RetType ret = CALL(writeReg(offsetXReg, xOffBuff, 2));
//        if (RET_SUCCESS != ret) {
//            RESET();
//            return ret;
//        }
//        ret = CALL(writeReg(offsetYReg, yOffBuff, 2));
//        if (RET_SUCCESS != ret) {
//            RESET();
//            return ret;
//        }
//
//        ret = CALL(writeReg(offsetZReg, zOffBuff, 2));

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
    uint8_t m_buff[6];

    /**
     * Reads the chip ID from the sensor
     * @param id
     * @return
     */
    RetType checkID() {
        RESUME();

        constexpr uint8_t chip_id = 0xE5;

        RetType ret = CALL(readReg(0x00, m_buff, 1));
        if (chip_id != m_buff[0]) ret = RET_ERROR;

        RESET();
        return ret;
    }

    /**
     * @brief Read registers from the sensor
     * @param command - register to read from
     * @param value - values to put data into
     * @param len - Registers to read from
     * @return
     */
    RetType readReg(uint8_t command, uint8_t *value, size_t len = 1) {
        RESUME();

        i2cAddr.mem_addr = command;
        RetType ret = CALL(m_i2c->read(i2cAddr, value, len));

        RESET();
        return ret;
    }

    /**
     * @brief Write values to to the sensor's registers
     * @param command - register to write to
     * @param value - value to write
     * @param len - Registers to write to
     * @return
     */
    RetType writeReg(uint8_t command, uint8_t *value, size_t len = 1) {
        RESUME();

        i2cAddr.mem_addr = command;
        RetType ret = CALL(m_i2c->write(i2cAddr, value, len));

        RESET();
        return ret;
    }

};


#endif //LAUNCH_CORE_ADXL375_H
