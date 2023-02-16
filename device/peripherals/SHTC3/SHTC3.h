/**
 * Platform Independent Driver for the SHTC3 Sensor
 *
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_SHTC3_H
#define LAUNCH_CORE_SHTC3_H

#include "device/I2CDevice.h"
#include "return.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "sched/macros/resume.h"

#define SHTC3_I2C_ADDR 0x70

enum SHTC3_CMD {
    SLEEP_CMD = 0xB098,
    WAKEUP_CMD = 0x3517,
    RESET_CMD = 0x805D,

    READ_ID_CMD = 0xEFC8,

    NORMAL_POW_MEAS_TEMP = 0x7866,
    LOW_POW_MEAS_TEMP = 0x609C,
    NORMAL_POW_MEAS_HUM = 0x58E0,
    LOW_POW_MEAS_HUM = 0x401A,

    NORMAL_POW_MEAS_TEMP_STRETCH = 0x7CA2,
    LOW_POW_MEAS_TEMP_STRETCH = 0x6458,
    NORMAL_POW_MEAS_HUM_STRETCH = 0x5C24,
    LOW_POW_MEAS_HUM_STRETCH = 0x44DE,
};

class SHTC3 {
   public:
    // TODO: Validate addr
    SHTC3(I2CDevice *i2CDevice) : mI2C(i2CDevice), inLowPowerMode(true), addr({.dev_addr = SHTC3_I2C_ADDR, .mem_addr = 0, .mem_addr_size = 0}) {}

    RetType init() {
        RESUME();

        uint16_t id = 0;
        RetType ret = CALL(getID(&id));
        if (ret != RET_SUCCESS) return ret;

        if ((id & 0x083F) != 0x807) {
            return RET_ERROR;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType getHumidityAndTemp(float *temperature, float *humidity) {
        RESUME();

        RetType ret;
        uint8_t buff[6];
        if (inLowPowerMode) {
            ret = CALL(writeCommand(LOW_POW_MEAS_TEMP));
            // TODO: Call a delay of 1
        } else {
            ret = CALL(writeCommand(NORMAL_POW_MEAS_TEMP));
            // TODO: Call a delay of 13
        }
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(mI2C->read(addr, buff, sizeof(buff)));
        if (ret != RET_SUCCESS) return ret;

        if ((buff[2] != crc8(buff, 2)) || (buff[5] != crc8(buff + 3, 2))) {
            return RET_ERROR;
        }

        int32_t calcTemp = static_cast<int32_t>((static_cast<uint32_t>(buff[0]) << 8) | buff[1]);
        calcTemp = ((4375 * calcTemp) >> 14) - 4500;
        *temperature = static_cast<float>(calcTemp) / 100.0f;

        uint32_t calcHum = (static_cast<uint32_t>(buff[3]) << 8) | buff[4];
        calcHum = (625 * calcHum) >> 12;
        *humidity = static_cast<float>(calcHum) / 100.0f;

        RESET();
        return RET_SUCCESS;
    }

    RetType toggleSleep(bool setSleep) {
        RESUME();

        RetType ret;
        if (setSleep) {
            ret = CALL(writeCommand(SLEEP_CMD));
        } else {
            ret = CALL(writeCommand(WAKEUP_CMD));
            // TODO: Delay for 200 microsecs in the future
        }

        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType reset() {
        RESUME();

        RetType ret = CALL(writeCommand(RESET_CMD));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType writeCommand(SHTC3_CMD command16) {
        RESUME();

        addr.reg_addr = SHTC3_I2C_ADDR << 1
        RetType ret = CALL(mI2C->write(addr, (uint8_t *) command16, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType readCommand(SHTC3_CMD command16, uint8_t *buff, uint8_t numBytes) {
        RESUME();


        addr.reg_addr = SHTC3_I2C_ADDR << 1
        RetType ret = CALL(mI2C->write(addr, (uint8_t *) command16, 2));
        if (ret != RET_SUCCESS) return ret;

        addr.reg_addr = (SHTC3_I2C_ADDR << 1) | 0x01
        ret = CALL(mI2C->read(addr, buff, numBytes));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getID(uint16_t *id) {
        RESUME();

        uint8_t data[2] = {};
        RetType ret = CALL(readCommand(READ_ID_CMD, data, 2));
        if (ret != RET_SUCCESS) return ret;

        *id = data[0] << 8 | data[1];

        RESET();
        return RET_SUCCESS;
    }

    RetType setPowerMode(bool lowPowerMode) {
        this->inLowPowerMode = lowPowerMode;
    }

   private:
    I2CDevice *mI2C;
    I2CAddr_t addr;
    bool inLowPowerMode;

    void uint16ToUint8(uint16_t data16, uint8_t *data8) {
        data8[0] = static_cast<uint8_t>(data16 >> 8);
        data8[1] = static_cast<uint8_t>(data16 & 0xFF);
    }

    static uint8_t crc8(const uint8_t *data, int len) {
        const uint8_t POLYNOMIAL(0x31);
        uint8_t crc(0xFF);

        for (int i = len; i; --i) {
            crc ^= *data++;

            for (int j = 8; j; --j) {
                crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
            }
        }
        return crc;
    }
};

#endif  // LAUNCH_CORE_SHTC3_H

