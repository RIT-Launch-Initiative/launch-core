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
    SHTC3(I2CDevice &i2CDevice) : mI2C(i2CDevice), inLowPowerMode(false), addr({.dev_addr = SHTC3_I2C_ADDR << 1, .mem_addr = 0, .mem_addr_size = 2}) {}

    RetType init() {
        RESUME();

        RetType ret = CALL(toggleSleep(false));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(reset());
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(getID(&this->id));
        if (ret != RET_SUCCESS) return ret;

        if ((this->id & 0x083F) != 0x807) {
            return RET_ERROR;
        }

        if (inLowPowerMode) {
            ret = CALL(writeCommand(LOW_POW_MEAS_TEMP));
            SLEEP(1000);
        } else {
            ret = CALL(writeCommand(NORMAL_POW_MEAS_TEMP));
            SLEEP(1000);
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType getHumidityAndTemp(float *temperature, float *humidity) {
        RESUME();

        uint16_t rawTemp;
        uint16_t rawHumidity;

        RetType ret = CALL(writeCommand(NORMAL_POW_MEAS_HUM_STRETCH));  // hang here fixed

        uint8_t buffer[2] = {};

        ret = CALL(mI2C.read(addr, buffer, 2));  // hanging here
        if (ret != RET_SUCCESS) return ret;
        rawHumidity = (buffer[0] << 8) | buffer[1];
        *humidity = calcHumidity(rawHumidity);

        ret = CALL(mI2C.read(addr, buffer, 2));
        if (ret != RET_SUCCESS) return ret;
        rawTemp = (buffer[0] << 8) | buffer[1];
        *temperature = calcTemp(rawTemp);

        RESET();
        return RET_SUCCESS;
    }

    static float calcTemp(uint16_t rawValue) {
        return 175 * static_cast<float>(rawValue) / 65536.0f - 45.0f;
    }

    static float calcHumidity(uint16_t rawValue) {
        return 100 * static_cast<float>(rawValue) / 65536.0f;
    }

    RetType toggleSleep(bool setSleep) {
        RESUME();

        static RetType ret;
        if (setSleep) {
            ret = CALL(writeCommand(SLEEP_CMD));
        } else {
            ret = CALL(writeCommand(WAKEUP_CMD));
            SLEEP(1);
        }

        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType reset() {
        RESUME();

        RetType ret = CALL(writeCommand(RESET_CMD));
        if (ret != RET_SUCCESS) return ret;

        SLEEP(100);

        RESET();
        return RET_SUCCESS;
    }

    RetType startWrite() {
    }

    RetType writeCommand(SHTC3_CMD command16) {
        RESUME();
        addr.dev_addr = (SHTC3_I2C_ADDR << 1);

        uint8_t command8[2];
        uint16ToUint8(command16, command8);
        RetType ret = CALL(mI2C.transmit(addr, command8, 2, 80));
        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType readCommand(SHTC3_CMD command16, uint8_t *buff, uint8_t numBytes) {
        RESUME();

        //        RetType ret = CALL(mI2C.transmit(addr, reinterpret_cast<uint8_t *>(&command16), 0));
        //        if (ret != RET_SUCCESS) return ret;
        //
        //        static uint8_t secondAddr |= 0x01 << 0;
        //        ret = CALL(mI2C.receive(addr, buff, numBytes));
        //        if (ret != RET_SUCCESS) return ret;
        uint16ToUint8(static_cast<uint16_t>(command16), buff);
        static uint8_t secondAddr = addr.dev_addr;
        secondAddr |= 0x01 << 0;

        RetType ret = CALL(mI2C.transmitReceive(addr, buff, 2, numBytes, 50, secondAddr));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getID(uint16_t *id) {
        RESUME();

        static uint8_t data[2] = {};
        RetType ret = CALL(readCommand(READ_ID_CMD, data, 2));
        if (ret != RET_SUCCESS) return ret;

        *id = data[0] << 8 | data[1];

        RESET();
        return RET_SUCCESS;
    }

    //    RetType setPowerMode(bool lowPowerMode) {
    //        this->inLowPowerMode = lowPowerMode;
    //    }

   private:
    I2CDevice &mI2C;
    I2CAddr_t addr;
    bool inLowPowerMode;
    uint16_t id;

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
