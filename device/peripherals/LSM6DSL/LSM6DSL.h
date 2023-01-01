/**
 * LSM6DSL IMU Facade
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LSM6DSL_H
#define LAUNCH_CORE_LSM6DSL_H

#include <stdint.h>
#include "device/I2CDevice.h"
#include "sched/macros.h"
#include "device/peripherals/LSM6DSL/LSM6DSL_Driver.h"

class LSM6DSL {
public:
    LSM6DSL(I2CDevice* i2CDevice) : mI2C(i2CDevice), xEnabled(false), gEnabled(false) {}

    RetType init() {
        RESUME();

        RetType ret = writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_IF_INC_ENABLED), 1, LSM6DSL_ACC_GYRO_IF_INC_MASK);
        if (ret != RET_SUCCESS) return ret;

        ret = writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_BDU_BLOCK_UPDATE), 1, LSM6DSL_ACC_GYRO_BDU_MASK);
        if (ret != RET_SUCCESS) return ret;


        ret = writeReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS), 1, LSM6DSL_ACC_GYRO_FIFO_MODE_MASK);
        if (ret != RET_SUCCESS) return ret;

        ret = writeReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN), 1, LSM6DSL_ACC_GYRO_ODR_FIFO_MASK);
        if (ret != RET_SUCCESS) return ret;




        ret = writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_IF_INC_ENABLED), 1, LSM6DSL_ACC_GYRO_IF_INC_MASK);
        if (ret != RET_SUCCESS) return ret;

        ret = writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_IF_INC_ENABLED), 1, LSM6DSL_ACC_GYRO_IF_INC_MASK);
        if (ret != RET_SUCCESS) return ret;


        RESET();
        return RET_SUCCESS;
    }

private:
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;
    bool xEnabled;
    bool gEnabled;

    RetType readReg(uint8_t reg, uint8_t *buff, size_t len, uint8_t mask) {
        RESUME();

        RetType ret = CALL(mI2C->read(i2cAddr, buff, len));
        if (ret != RET_SUCCESS) return ret;

        *buff &= mask;

        RESET();
        return RET_SUCCESS;
    }

    RetType writeReg(uint8_t reg, uint8_t *buff, size_t len, uint8_t mask) {
        RESUME();

        uint8_t value;
        i2cAddr.mem_addr = reg;

        RetType ret = CALL(mI2C->read(i2cAddr, &value, len));
        if (ret != RET_SUCCESS) return ret;

        value &= ~mask;
        value |= *buff;

        ret = CALL(mI2C->write(i2cAddr, &value, len));

        RESET();
        return RET_SUCCESS;
    }



};

#endif //LAUNCH_CORE_LSM6DSL_H
