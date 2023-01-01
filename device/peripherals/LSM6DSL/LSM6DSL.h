/**
 * LSM6DSL IMU
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LSM6DSL_H
#define LAUNCH_CORE_LSM6DSL_H

#define LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G   0.061  /**< Sensitivity value for 2 g full scale [mg/LSB] */
#define LSM6DSL_ACC_SENSITIVITY_FOR_FS_4G   0.122  /**< Sensitivity value for 4 g full scale [mg/LSB] */
#define LSM6DSL_ACC_SENSITIVITY_FOR_FS_8G   0.244  /**< Sensitivity value for 8 g full scale [mg/LSB] */
#define LSM6DSL_ACC_SENSITIVITY_FOR_FS_16G  0.488  /**< Sensitivity value for 16 g full scale [mg/LSB] */

#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_125DPS   04.375  /**< Sensitivity value for 125 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_245DPS   08.750  /**< Sensitivity value for 245 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_500DPS   17.500  /**< Sensitivity value for 500 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_1000DPS  35.000  /**< Sensitivity value for 1000 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_FOR_FS_2000DPS  70.000  /**< Sensitivity value for 2000 dps full scale [mdps/LSB] */

#define LSM6DSL_PEDOMETER_THRESHOLD_LOW       0x00  /**< Lowest  value of pedometer threshold */
#define LSM6DSL_PEDOMETER_THRESHOLD_MID_LOW   0x07
#define LSM6DSL_PEDOMETER_THRESHOLD_MID       0x0F
#define LSM6DSL_PEDOMETER_THRESHOLD_MID_HIGH  0x17
#define LSM6DSL_PEDOMETER_THRESHOLD_HIGH      0x1F  /**< Highest value of pedometer threshold */

#define LSM6DSL_WAKE_UP_THRESHOLD_LOW       0x01  /**< Lowest  value of wake up threshold */
#define LSM6DSL_WAKE_UP_THRESHOLD_MID_LOW   0x0F
#define LSM6DSL_WAKE_UP_THRESHOLD_MID       0x1F
#define LSM6DSL_WAKE_UP_THRESHOLD_MID_HIGH  0x2F
#define LSM6DSL_WAKE_UP_THRESHOLD_HIGH      0x3F  /**< Highest value of wake up threshold */

#define LSM6DSL_TAP_THRESHOLD_LOW       0x01  /**< Lowest  value of wake up threshold */
#define LSM6DSL_TAP_THRESHOLD_MID_LOW   0x08
#define LSM6DSL_TAP_THRESHOLD_MID       0x10
#define LSM6DSL_TAP_THRESHOLD_MID_HIGH  0x18
#define LSM6DSL_TAP_THRESHOLD_HIGH      0x1F  /**< Highest value of wake up threshold */

#define LSM6DSL_TAP_SHOCK_TIME_LOW       0x00  /**< Lowest  value of wake up threshold */
#define LSM6DSL_TAP_SHOCK_TIME_MID_LOW   0x01
#define LSM6DSL_TAP_SHOCK_TIME_MID_HIGH  0x02
#define LSM6DSL_TAP_SHOCK_TIME_HIGH      0x03  /**< Highest value of wake up threshold */

#define LSM6DSL_TAP_QUIET_TIME_LOW       0x00  /**< Lowest  value of wake up threshold */
#define LSM6DSL_TAP_QUIET_TIME_MID_LOW   0x01
#define LSM6DSL_TAP_QUIET_TIME_MID_HIGH  0x02
#define LSM6DSL_TAP_QUIET_TIME_HIGH      0x03  /**< Highest value of wake up threshold */

#define LSM6DSL_TAP_DURATION_TIME_LOW       0x00  /**< Lowest  value of wake up threshold */
#define LSM6DSL_TAP_DURATION_TIME_MID_LOW   0x04
#define LSM6DSL_TAP_DURATION_TIME_MID       0x08
#define LSM6DSL_TAP_DURATION_TIME_MID_HIGH  0x0C
#define LSM6DSL_TAP_DURATION_TIME_HIGH      0x0F  /**< Highest value of wake up threshold */

#include <stdint.h>
#include "device/I2CDevice.h"
#include "sched/macros.h"
#include "device/peripherals/LSM6DSL/LSM6DSL_Driver.h"

class LSM6DSL {
public:
    LSM6DSL(I2CDevice *i2CDevice) : mI2C(i2CDevice), accelEnabled(false), gyroEnabled(false) {}

    RetType init(uint8_t i2cDevAddr) {
        RESUME();
        i2cAddr = {
                .dev_addr = i2cDevAddr,
                .mem_addr = LSM6DSL_ACC_GYRO_WHO_AM_I_REG,
                .mem_addr_size = 1
        };

        // Check Chip ID
        uint8_t chipID;
        RetType ret = readReg(LSM6DSL_ACC_GYRO_WHO_AM_I_REG, &chipID, 1, LSM6DSL_ACC_GYRO_WHO_AM_I_BIT_MASK);
        if (ret != RET_SUCCESS || LSM6DSL_ACC_GYRO_WHO_AM_I != chipID) return ret;

        // Enable reg addr automatically incremented during multi byte access with serial intf
        ret = CALL(
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

        ret = CALL(setFullScaleAccel(2.0f));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, reinterpret_cast<uint8_t *>(LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN), 1,
                            LSM6DSL_ACC_GYRO_IF_INC_MASK));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setFullScaleGyro(2000.0f));
        if (ret != RET_SUCCESS) return ret;

        accelLastODR = 104.0f;
        accelEnabled = true;
        gyroLastODR = 104.0f;
        gyroEnabled = true;

        RESET();
        return RET_SUCCESS;
    }

    /**********************************************************
     * Acceleration Functions
     **********************************************************/

    RetType getAccelAxes(int32_t *accelData) {
        RESUME();

        int16_t rawData[3];
        float sens = 0;

        RetType ret = CALL(getAccelAxesRaw(rawData));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(getAccelSens(&sens));
        if (ret != RET_SUCCESS) return ret;


        for (int i = 0; i < 3; i++) {
            accelData[i] = static_cast<int32_t>(rawData[i] * sens);
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType getAccelAxesRaw(int16_t *accelData) {
        RESUME();

        uint8_t regValue[6] = {};

        int accelDataIndex;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; i < 3; i++) {
                RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_OUTX_L_XL + accelDataIndex,
                                           reinterpret_cast<uint8_t *>(&accelData[accelDataIndex]), 1));
                if (ret != RET_SUCCESS) return ret;

                accelDataIndex++;
            }
        }


        accelData[0] = static_cast<int16_t>(regValue[1] << 8) + static_cast<int16_t>(regValue[0]);
        accelData[1] = static_cast<int16_t>(regValue[3] << 8) + static_cast<int16_t>(regValue[2]);
        accelData[2] = static_cast<int16_t>(regValue[5] << 8) + static_cast<int16_t>(regValue[4]);

        RESET();
        return RET_SUCCESS;
    }

    RetType getAccelSens(float *sens) {
        RESUME();

        LSM6DSL_ACC_GYRO_FS_XL_t fullScale;

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_CTRL1_XL, reinterpret_cast<uint8_t *>(&fullScale), 1));
        if (ret != RET_SUCCESS) return ret;

        switch (fullScale) {
            case LSM6DSL_ACC_GYRO_FS_XL_2g:
                *sens = static_cast<float>(LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G);
                break;
            case LSM6DSL_ACC_GYRO_FS_XL_4g:
                *sens = static_cast<float>(LSM6DSL_ACC_SENSITIVITY_FOR_FS_4G);
                break;
            case LSM6DSL_ACC_GYRO_FS_XL_8g:
                *sens = static_cast<float>(LSM6DSL_ACC_SENSITIVITY_FOR_FS_8G);
                break;
            case LSM6DSL_ACC_GYRO_FS_XL_16g:
                *sens = static_cast<float>(LSM6DSL_ACC_SENSITIVITY_FOR_FS_16G);
                break;
            default:
                *sens = -1.0f;
                return RET_ERROR;
        }

        RESET();
        return RET_SUCCESS;
    }


    RetType setFullScaleAccel(float fullScale) {
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


    /**********************************************************
     * Gyroscope Functions
     **********************************************************/



    RetType setFullScaleGyro(float fullScale) {
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
    float accelLastODR;
    float gyroLastODR;
    bool accelEnabled;
    bool gyroEnabled;

    RetType readReg(uint8_t reg, uint8_t *buff, size_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;

        RetType ret = CALL(mI2C->read(i2cAddr, buff, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType writeReg(uint8_t reg, const uint8_t *buff, size_t len) {
        RESUME();

        uint8_t value;
        i2cAddr.mem_addr = reg;

        RetType ret = CALL(mI2C->read(i2cAddr, &value, len));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(mI2C->write(i2cAddr, &value, len));

        RESET();
        return RET_SUCCESS;
    }

    RetType readReg(uint8_t reg, uint8_t *buff, size_t len, uint8_t mask) {
        RESUME();

        i2cAddr.mem_addr = reg;

        RetType ret = CALL(mI2C->read(i2cAddr, buff, len));
        if (ret != RET_SUCCESS) return ret;

        *buff &= mask;

        RESET();
        return RET_SUCCESS;
    }

    RetType writeReg(uint8_t reg, const uint8_t *buff, size_t len, uint8_t mask) {
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
