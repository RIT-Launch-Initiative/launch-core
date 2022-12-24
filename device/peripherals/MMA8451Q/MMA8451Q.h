/**
 * MMA8451Q Platform Independent Driver Implementation Using launch-core
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
        uint8_t data[2] {CTRL_REG_1, 0x01};

        RetType ret = CALL(mI2C->write(addr, data, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

private:
    I2CDevice *mI2C;
    I2CAddr_t addr;

};

#endif //LAUNCH_CORE_MMA8451Q_H
