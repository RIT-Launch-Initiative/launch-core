/**
 * MMA8451Q Platform Independent Driver Implementation Using launch-core
 *
 * NOTE: Untested and most likely unused. Will be replaced by other sensors.
 *
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_MMA8451Q_H
#define LAUNCH_CORE_MMA8451Q_H

#include "device/I2CDevice.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "sched/macros/call.h"


#define MMA8451_I2C_ADDRESS (0x1d<<1)
#define UINT14_MAX 16383

enum MMA8451_REG {
    X_MSB_REG = 0x01,
    Y_MSB_REG = 0x03,
    Z_MSB_REG = 0x05,

    CTRL_REG_1 = 0x2A,
    DEVICE_ID_REG = 0x0D
};

class MMA8451Q {
public:
    MMA8451Q(I2CDevice *i2CDevice) : mI2C(i2CDevice) {}

    RetType init() {
        RESUME();
        // TODO: Figure mem and size out
        mAddr = {
                .dev_addr = MMA8451_I2C_ADDRESS,
                .mem_addr = 0,
                .mem_addr_size = 0
        };

        uint8_t data[2]{CTRL_REG_1, 0x01};

        RetType ret = CALL(mI2C->write(mAddr, data, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getAxisAccel(MMA8451_REG reg, int16_t *result16) {
        RESUME();
        uint8_t result8[2] = {};

        RetType ret = CALL(readReg(reg, result8, 2));
        if (ret != RET_SUCCESS) return ret;

        *result16 = static_cast<int16_t>((result8[0] << 6) | (result8[1] >> 2));
        if (*result16 > UINT14_MAX / 2) {
            *result16 -= UINT14_MAX;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType getXAccel(int16_t *result) {
        RESUME();

        RetType ret = CALL(getAxisAccel(X_MSB_REG, result));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getYAccel(int16_t *result) {
        RESUME();

        RetType ret = CALL(getAxisAccel(Y_MSB_REG, result));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getZAccel(int16_t *result) {
        RESUME();

        RetType ret = CALL(getAxisAccel(Z_MSB_REG, result));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getAllAccel(int16_t *result) {
        RESUME();

        RetType ret = CALL(getXAccel(&result[0]));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(getXAccel(&result[1]));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(getXAccel(&result[2]));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    uint8_t getDeviceID(uint8_t *id) {
        RESUME();

        RetType ret = CALL(readReg(DEVICE_ID_REG, id, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }


private:
    I2CDevice *mI2C;
    I2CAddr_t mAddr;


    RetType readReg(MMA8451_REG addr, uint8_t *data, int len) {
        RESUME();

        uint8_t regAddr[1] = {static_cast<uint8_t>(addr)};
        RetType ret = CALL(mI2C->write(mAddr, regAddr, 1));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(mI2C->read(mAddr, data, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType writeReg(uint8_t *data, int len) {
        RESUME();

        RetType ret = CALL(mI2C->write(mAddr, data, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

};

#endif //LAUNCH_CORE_MMA8451Q_H
