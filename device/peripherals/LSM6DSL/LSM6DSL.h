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
    LSM6DSL(I2CDevice *i2CDevice) : mI2C(i2CDevice), xEnabled(false), gEnabled(false) {}

    RetType init(uint8_t i2cDevAddr) {
        RESUME();
        i2cAddr = {
                .dev_addr = i2cDevAddr,
                .mem_addr = LSM6DSL_ACC_GYRO_WHO_AM_I_REG,
                .mem_addr_size = 1
        };

        // Enable reg addr automatically incremented during multi byte access with serial intf
        RetType ret = CALL(
                writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_IF_INC_ENABLED), 1,
                         LSM6DSL_ACC_GYRO_IF_INC_MASK));
        if (ret != RET_SUCCESS) return ret;

        // Enable BDU
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_BDU_BLOCK_UPDATE), 1,
                            LSM6DSL_ACC_GYRO_BDU_MASK));
        if (ret != RET_SUCCESS) return ret;

        // FIFO Mode Select
        ret = CALL(
                writeReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS), 1,
                         LSM6DSL_ACC_GYRO_FIFO_MODE_MASK));
        if (ret != RET_SUCCESS) return ret;

        // Output Data Rate Selection
        ret = CALL(
                writeReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN),
                         1, LSM6DSL_ACC_GYRO_ODR_FIFO_MASK));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setFullScaleX(2.0f));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN), 1,
                            LSM6DSL_ACC_GYRO_IF_INC_MASK));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setFullScaleG(2000.0f));
        if (ret != RET_SUCCESS) return ret;

        xLastODR = 104.0f;
        xEnabled = 0;
        gLastODR = 104.0f;
        gEnabled = 0;

        RESET();
        return RET_SUCCESS;
    }

    RetType setFullScaleX(float fullScale) {
        RESUME();

        LSM6DSL_ACC_GYRO_FS_XL_t newFs = (fullScale <= 2.0f) ? LSM6DSL_ACC_GYRO_FS_XL_2g : (fullScale <= 4.0f)
                                                                                           ? LSM6DSL_ACC_GYRO_FS_XL_4g
                                                                                           : (fullScale <= 8.0f)
                                                                                             ? LSM6DSL_ACC_GYRO_FS_XL_8g
                                                                                             : LSM6DSL_ACC_GYRO_FS_XL_16g;

        RetType ret = CALL(
                writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, reinterpret_cast<uint8_t *>(&newFs), 1, LSM6DSL_ACC_GYRO_CTRL1_XL));
        if (ret != RET_SUCCESS) return ret;


        RESET();
        return RET_SUCCESS;
    }

    RetType setFullScaleG(float fullScale) {
        RESUME();

        LSM6DSL_ACC_GYRO_FS_G_t newFs;

        if (fullScale <= 125.0f) {
            writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_FS_125_ENABLED), 1,
                     LSM6DSL_ACC_GYRO_FS_125_MASK);
        } else {
            newFs = (fullScale <= 245.0f) ? LSM6DSL_ACC_GYRO_FS_G_245dps
                                          : (fullScale <= 500.0f) ? LSM6DSL_ACC_GYRO_FS_G_500dps
                                                                  : (fullScale <= 1000.0f)
                                                                    ? LSM6DSL_ACC_GYRO_FS_G_1000dps
                                                                    : LSM6DSL_ACC_GYRO_FS_G_2000dps;
            RetType ret = writeReg(LSM6DSL_ACC_GYRO_CTRL2_G,
                                   reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_FS_125_DISABLED), 1,
                                   LSM6DSL_ACC_GYRO_FS_125_MASK);
            if (ret != RET_SUCCESS) return ret;

            ret = writeReg(LSM6DSL_ACC_GYRO_CTRL2_G,
                           reinterpret_cast<uint8_t *>(newFs), 1,
                           LSM6DSL_ACC_GYRO_FS_G_MASK);
            if (ret != RET_SUCCESS) return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

private:
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;
    float xLastODR;
    float gLastODR;
    bool xEnabled;
    bool gEnabled;


    RetType readReg(uint8_t reg, uint8_t *buff, size_t len, uint8_t mask) {
        RESUME();

        i2cAddr.mem_addr = reg;

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
