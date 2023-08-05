/**
 * Platform Independent LSM6DSL IMU Driver Using scheduler
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LSM6DSL_H
#define LAUNCH_CORE_LSM6DSL_H


#define LSM6DSL_DATA_STRUCT(variable_name) LSM6DSL_DATA_T variable_name = {.id = 11000, .x_accel = 0, .x_gyro = 0, .y_accel = 0, .y_gyro = 0, .z_accel = 0, .z_gyro = 0}
#define ERROR_CHECK(ret) if (ret != RET_SUCCESS) {RESET(); return ret;}


#include <stdint.h>
#include "device/I2CDevice.h"
#include "sched/macros.h"
#include "device/peripherals/LSM6DSL/LSM6DSL_Driver.h"

typedef struct {
    const uint16_t id;
    int32_t x_accel;
    int32_t x_gyro;
    int32_t y_accel;
    int32_t y_gyro;
    int32_t z_accel;
    int32_t z_gyro;
} LSM6DSL_DATA_T;

class LSM6DSL : public Device {
public:
    enum LSM6DSL_Interrupt_Pin_t {
        LSM6DSL_INT1_PIN,
        LSM6DSL_INT2_PIN
    };

    enum LSM6DSL_I2C_ADDR {
        LSM6DSL_I2C_ADDR_PRIMARY = 0x6A,
        LSM6DSL_I2C_ADDR_SECONDARY = 0x6B
    };

    static constexpr float LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G = 0.061;  /**< Sensitivity value for 2 g full scale [mg/LSB] */
    static constexpr float LSM6DSL_ACC_SENSITIVITY_FOR_FS_4G = 0.122;  /**< Sensitivity value for 4 g full scale [mg/LSB] */
    static constexpr float LSM6DSL_ACC_SENSITIVITY_FOR_FS_8G = 0.244;  /**< Sensitivity value for 8 g full scale [mg/LSB] */
    static constexpr float LSM6DSL_ACC_SENSITIVITY_FOR_FS_16G = 0.488;  /**< Sensitivity value for 16 g full scale [mg/LSB] */

    static constexpr float LSM6DSL_GYRO_SENSITIVITY_FOR_FS_125DPS = 04.375;  /**< Sensitivity value for 125 dps full scale [mdps/LSB] */
    static constexpr float LSM6DSL_GYRO_SENSITIVITY_FOR_FS_245DPS = 08.750;  /**< Sensitivity value for 245 dps full scale [mdps/LSB] */
    static constexpr float LSM6DSL_GYRO_SENSITIVITY_FOR_FS_500DPS = 17.500;  /**< Sensitivity value for 500 dps full scale [mdps/LSB] */
    static constexpr float LSM6DSL_GYRO_SENSITIVITY_FOR_FS_1000DPS = 35.000;  /**< Sensitivity value for 1000 dps full scale [mdps/LSB] */
    static constexpr float LSM6DSL_GYRO_SENSITIVITY_FOR_FS_2000DPS = 70.000;  /**< Sensitivity value for 2000 dps full scale [mdps/LSB] */

    static constexpr uint8_t LSM6DSL_WAKE_UP_THRESHOLD_LOW = 0x01;  /**< Lowest  value of wake up threshold */
    static constexpr uint8_t LSM6DSL_WAKE_UP_THRESHOLD_MID_LOW = 0x0F;
    static constexpr uint8_t LSM6DSL_WAKE_UP_THRESHOLD_MID = 0x1F;
    static constexpr uint8_t LSM6DSL_WAKE_UP_THRESHOLD_MID_HIGH = 0x2F;
    static constexpr uint8_t LSM6DSL_WAKE_UP_THRESHOLD_HIGH = 0x3F;  /**< Highest value of wake up threshold */

    static constexpr float GRAVITY = 9.80665f;

    LSM6DSL(I2CDevice &i2CDevice, uint16_t address = LSM6DSL_I2C_ADDR_SECONDARY, const char *name = "LSM6DSL") : Device(name), m_i2c(&i2CDevice),
    m_i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

    RetType init() {
        RESUME();
        m_i2cAddr.mem_addr = LSM6DSL_ACC_GYRO_WHO_AM_I_REG;

        RetType ret = CALL(checkChipID());
        ERROR_CHECK(ret);

        // Enable reg addr automatically incremented during multi byte access with serial intf
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, LSM6DSL_ACC_GYRO_IF_INC_ENABLED, 1, LSM6DSL_ACC_GYRO_IF_INC_MASK));
        ERROR_CHECK(ret);

        // Enable BDU
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, LSM6DSL_ACC_GYRO_BDU_BLOCK_UPDATE, 1, LSM6DSL_ACC_GYRO_BDU_MASK));
        ERROR_CHECK(ret);

        // FIFO Mode Select
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS, 1,
                            LSM6DSL_ACC_GYRO_FIFO_MODE_MASK));
        ERROR_CHECK(ret);

        // ODR Selection
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_ACC_GYRO_ODR_G_104Hz, 1, LSM6DSL_ACC_GYRO_ODR_G_MASK));
        ERROR_CHECK(ret);

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_GYRO_ODR_XL_104Hz, 1, LSM6DSL_ACC_GYRO_ODR_XL_MASK));
        ERROR_CHECK(ret);

        // Full Scale
        ret = CALL(setAccelFullScale(LSM6DSL_ACC_GYRO_FS_XL_2g));
        ERROR_CHECK(ret);

        ret = CALL(setGyroFullScale(2000.0f));
        ERROR_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }


    RetType getData(LSM6DSL_DATA_T *data) {
        RESUME();

        RetType ret = CALL(getAccelAxesMS2(&data->x_accel, &data->y_accel, &data->z_accel));
        if (RET_SUCCESS == ret) {
            ret = CALL(getGyroAxes(&data->x_gyro, &data->y_gyro, &data->z_gyro));
        }

        RESET();
        return ret;
    }

    /**********************************************************
     * Acceleration Functions
     **********************************************************/

    RetType getAccelAxesMS2(int32_t *accelX, int32_t *accelY, int32_t *accelZ) {
        RESUME();

        RetType ret = CALL(getAccelAxes(accelX, accelY, accelZ));
        if (RET_SUCCESS == ret) {
            *accelX *= GRAVITY;
            *accelY *= GRAVITY;
            *accelZ *= GRAVITY;

            *accelX /= 1000;
            *accelY /= 1000;
            *accelZ /= 1000;
        }

        RESET();
        return ret;
    }

    RetType getAccelAxes(int32_t *accelX, int32_t *accelY, int32_t *accelZ) {
        RESUME();

        static float sens = 0;

        RetType ret = CALL(getAccelAxesRaw(m_buff));
        ERROR_CHECK(ret);

        ret = CALL(getAccelSens(&sens));
        ERROR_CHECK(ret);

        *accelX = static_cast<int32_t>((m_buff[0] << 8 | m_buff[1]) * sens);
        *accelY = static_cast<int32_t>((m_buff[2] << 8 | m_buff[3]) * sens);
        *accelZ = static_cast<int32_t>((m_buff[4] << 8 | m_buff[5]) * sens);

        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Get raw acceleration values from sensor's regist
     * @param accelData - Pointer to array of 6 bytes
     * @return RetType - Scheduler status
     */
    RetType getAccelAxesRaw(uint8_t *accelData) {
        RESUME();

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_OUTX_L_XL, accelData, 6));

        RESET();
        return ret;
    }

    RetType getAccelSens(float *sens) {
        RESUME();

        static LSM6DSL_ACC_GYRO_FS_XL_t fullScale;

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_CTRL1_XL, reinterpret_cast<uint8_t *>(&fullScale), 1,
                                   LSM6DSL_ACC_GYRO_FS_XL_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

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


    RetType setAccelFullScale(LSM6DSL_ACC_GYRO_FS_XL_t fullScale) {
        RESUME();

        m_buff[0] = fullScale;
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, m_buff[0], 1, LSM6DSL_ACC_GYRO_FS_XL_MASK));

        RESET();
        return ret;
    }

    RetType setAccelODR(LSM6DSL_ACC_GYRO_ODR_XL_t odr) {
        RESUME();

        m_buff[0] = odr;
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, m_buff[0], 1, LSM6DSL_ACC_GYRO_ODR_XL_MASK));

        RESET();
        return ret;
    }

    /**********************************************************
     * Gyroscope Functions
     **********************************************************/
    RetType getGyroAxes(int32_t *gyroX, int32_t *gyroY, int32_t *gyroZ) {
        RESUME();

        static int16_t rawData[3];
        static float sens = 0;

        RetType ret = CALL(getGyroAxesRaw(rawData));
        ERROR_CHECK(ret);

        ret = CALL(getGyroSens(&sens));
        ERROR_CHECK(ret);

        *gyroX = static_cast<int32_t>(rawData[0] * sens);
        *gyroY = static_cast<int32_t>(rawData[1] * sens);
        *gyroZ = static_cast<int32_t>(rawData[2] * sens);

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroAxesRaw(int16_t *gyroData) {
        RESUME();

        static uint8_t regValue[6] = {};

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_OUTX_L_G, regValue, 6));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
        gyroData[0] = static_cast<int16_t>(regValue[1] << 8) | static_cast<int16_t>(regValue[0]);
        gyroData[1] = static_cast<int16_t>(regValue[3] << 8) | static_cast<int16_t>(regValue[2]);
        gyroData[2] = static_cast<int16_t>(regValue[5] << 8) | static_cast<int16_t>(regValue[4]);

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroSens(float *sens) {
        RESUME();

        static LSM6DSL_ACC_GYRO_FS_125_t fullScale125;
        static LSM6DSL_ACC_GYRO_FS_G_t fullScale;

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_CTRL2_G, reinterpret_cast<uint8_t *>(&fullScale125), 1,
                                   LSM6DSL_ACC_GYRO_FS_125_MASK));


        if (fullScale125 == LSM6DSL_ACC_GYRO_FS_125_ENABLED) {
            *sens = static_cast<float>(LSM6DSL_GYRO_SENSITIVITY_FOR_FS_125DPS);
        } else {
            ret = CALL(readReg(LSM6DSL_ACC_GYRO_CTRL2_G, reinterpret_cast<uint8_t *>(&fullScale), 1,
                               LSM6DSL_ACC_GYRO_FS_G_MASK));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            switch (fullScale) {
                case LSM6DSL_ACC_GYRO_FS_G_245dps:
                    *sens = static_cast<float>(LSM6DSL_GYRO_SENSITIVITY_FOR_FS_245DPS);
                    break;
                case LSM6DSL_ACC_GYRO_FS_G_500dps:
                    *sens = static_cast<float>(LSM6DSL_GYRO_SENSITIVITY_FOR_FS_500DPS);
                    break;
                case LSM6DSL_ACC_GYRO_FS_G_1000dps:
                    *sens = static_cast<float>(LSM6DSL_GYRO_SENSITIVITY_FOR_FS_1000DPS);
                    break;
                case LSM6DSL_ACC_GYRO_FS_G_2000dps:
                    *sens = static_cast<float>(LSM6DSL_GYRO_SENSITIVITY_FOR_FS_2000DPS);
                    break;
                default:
                    *sens = -1.0f;
                    return RET_ERROR;
            }
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroFullScale(float fullScale) {
        RESUME();

        static LSM6DSL_ACC_GYRO_FS_G_t newFs;

        if (fullScale <= 125.0f) {
            RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_ACC_GYRO_FS_125_ENABLED, 1,
                                        LSM6DSL_ACC_GYRO_FS_125_MASK));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

        } else {
            newFs = (fullScale <= 245.0f) ? LSM6DSL_ACC_GYRO_FS_G_245dps
                                          : (fullScale <= 500.0f) ? LSM6DSL_ACC_GYRO_FS_G_500dps
                                                                  : (fullScale <= 1000.0f)
                                                                    ? LSM6DSL_ACC_GYRO_FS_G_1000dps
                                                                    : LSM6DSL_ACC_GYRO_FS_G_2000dps;
            RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_ACC_GYRO_FS_125_DISABLED, 1,
                                        LSM6DSL_ACC_GYRO_FS_125_MASK));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, newFs, 1, LSM6DSL_ACC_GYRO_FS_G_MASK));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroODR(LSM6DSL_ACC_GYRO_ODR_G_t odr) {
        RESUME();

        m_buff[0] = odr;
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, m_buff[0], 1, LSM6DSL_ACC_GYRO_ODR_G_MASK));

        RESET();
        return ret;
    }

    /********************************************************************
     * Free Fall Settings
     ********************************************************************/

    RetType enableFreeFallDetection(LSM6DSL_Interrupt_Pin_t interruptPin) {
        RESUME();

        RetType ret = CALL(setAccelODR(LSM6DSL_ACC_GYRO_ODR_XL_416Hz));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Full Scale Select
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_GYRO_FS_XL_2g, 1, LSM6DSL_ACC_GYRO_FS_XL_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // FF Duration
        uint8_t duration = 0x06;
        uint8_t lowVal = duration & 0x1F;
        lowVal = lowVal << LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_POSITION;
        lowVal &= LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_FREE_FALL, lowVal, 1, LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        uint8_t highVal = (duration >> 5) & 0x1;
        highVal = highVal << LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_POSITION;
        highVal &= LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, highVal, 1, LSM6DSL_ACC_GYRO_FS_XL_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Wake Duration
        duration = 0x00 << LSM6DSL_ACC_GYRO_WAKE_DUR_POSITION;
        duration &= LSM6DSL_ACC_GYRO_WAKE_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, duration, 1, LSM6DSL_ACC_GYRO_WAKE_DUR_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Timer HR
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, LSM6DSL_ACC_GYRO_TIMER_HR_6_4ms, 1,
                            LSM6DSL_ACC_GYRO_TIMER_HR_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Sleep Duration
        duration = 0x00 << LSM6DSL_ACC_GYRO_SLEEP_DUR_POSITION;
        duration &= LSM6DSL_ACC_GYRO_SLEEP_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, duration, 1, LSM6DSL_ACC_GYRO_SLEEP_DUR_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Free Fall THS
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_GYRO_FF_THS_312mg, 1, LSM6DSL_ACC_GYRO_FF_THS_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Enable Interrupts
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_CFG1, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED, 1,
                            LSM6DSL_ACC_GYRO_INT_EN_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        /* Enable free fall event on either INT1 or INT2 pin */
        switch (interruptPin) {
            case LSM6DSL_INT1_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_FF_ENABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT1_FF_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }

            case LSM6DSL_INT2_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT1_FF_ENABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT2_FF_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }

            default:
                return RET_ERROR;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType disableFreeFallDetection() {
        RESUME();

        // Disable FF events on pins
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_FF_DISABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT1_FF_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT1_FF_DISABLED, 1,
                            LSM6DSL_ACC_GYRO_INT2_FF_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Disable Interrupts
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_CFG1, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED, 1,
                            LSM6DSL_ACC_GYRO_INT_EN_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Set Free Fall Duration
        uint8_t duration = 0x00;

        uint8_t lowVal = duration & 0x1F;
        lowVal = lowVal << LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_POSITION;
        lowVal &= LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_FREE_FALL, lowVal, 1, LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        uint8_t highVal = (duration >> 5) & 0x1;
        highVal = highVal << LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_POSITION;
        highVal &= LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, highVal, 1, LSM6DSL_ACC_GYRO_FS_XL_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Set Free Fall THS Setting
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_GYRO_FF_THS_156mg, 1, LSM6DSL_ACC_GYRO_FF_THS_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType setFreeFallThreshold(uint8_t threshold) {
        RESUME();

        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_FREE_FALL, threshold, 1, LSM6DSL_ACC_GYRO_FF_THS_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    /********************************************************************
     * Tilt Detection Settings
     ********************************************************************/


    RetType enableTiltDetection(LSM6DSL_Interrupt_Pin_t interruptPin) {
        RESUME();

        // Output Data Rate
        RetType ret = CALL(setGyroODR(LSM6DSL_ACC_GYRO_ODR_G_26Hz));
        ERROR_CHECK(ret);

        // Full Scale Selection
        ret = CALL(setGyroFullScale(2.0f));
        ERROR_CHECK(ret);

        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Enable Embedded Functionalities
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL10_C, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED, 1,
                            LSM6DSL_ACC_GYRO_FUNC_EN_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Enable Tilt Calculation
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL10_C, LSM6DSL_ACC_GYRO_TILT_ENABLED, 1, LSM6DSL_ACC_GYRO_TILT_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Enable Tilt Detection
        switch (interruptPin) {
            case LSM6DSL_INT1_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_TILT_ENABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT1_TILT_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }
                break;

            case LSM6DSL_INT2_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT2_TILT_ENABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT2_TILT_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }
                break;

            default:
                return RET_ERROR;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType disableTiltDetection() {
        RESUME();

        // Disable Tilt Events
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_TILT_DISABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT1_TILT_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT2_TILT_DISABLED, 1,
                            LSM6DSL_ACC_GYRO_INT2_TILT_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Disable Tilt Calculations
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL10_C, LSM6DSL_ACC_GYRO_TILT_DISABLED, 1, LSM6DSL_ACC_GYRO_TILT_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Disable Embedded Functionalities
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL10_C, LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED, 1,
                            LSM6DSL_ACC_GYRO_FUNC_EN_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    /********************************************************************
     * Six Degree Orientation Settings
     ********************************************************************/

    RetType enable6DOrientation(LSM6DSL_Interrupt_Pin_t interruptPin) {
        RESUME();

        // Output Data Rate selection
        RetType ret = CALL(setAccelODR(LSM6DSL_ACC_GYRO_ODR_XL_416Hz));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(setAccelFullScale(LSM6DSL_ACC_GYRO_FS_XL_2g));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Set 6D Threshold
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, LSM6DSL_ACC_GYRO_SIXD_THS_60_degree, 1,
                            LSM6DSL_ACC_GYRO_SIXD_THS_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        /* Enable basic Interrupts */
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_CFG1, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED, 1,
                            LSM6DSL_ACC_GYRO_INT_EN_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Enable 6D orientation on either INT1 or INT2 pin
        switch (interruptPin) {
            case LSM6DSL_INT1_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_6D_ENABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT1_6D_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }

                break;
            case LSM6DSL_INT2_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT2_6D_ENABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT2_6D_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }

                break;
            default:
                return RET_ERROR;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType disableSixDoF() {
        RESUME();

        // Disable 6D Interrupts
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_6D_DISABLED, 1,
                                    LSM6DSL_ACC_GYRO_INT1_6D_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT2_6D_DISABLED, 1,
                            LSM6DSL_ACC_GYRO_INT2_6D_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Disable Basic Interrupts
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_CFG1, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED, 1,
                            LSM6DSL_ACC_GYRO_INT_EN_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        /* Reset 6D threshold. */
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, LSM6DSL_ACC_GYRO_SIXD_THS_80_degree, 1,
                            LSM6DSL_ACC_GYRO_SIXD_THS_MASK));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    // TODO: Maybe add wakeup detection functionality

    RetType poll() override {
        return RET_SUCCESS;
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }


private:
    I2CDevice *m_i2c;
    I2CAddr_t m_i2cAddr;
    uint8_t m_buff[10];
    uint8_t m_buffSecondary[5];

    RetType checkChipID() {
        RESUME();

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_WHO_AM_I_REG, m_buff, 1, LSM6DSL_ACC_GYRO_WHO_AM_I_BIT_MASK, 50));
        if (RET_SUCCESS == ret && LSM6DSL_ACC_GYRO_WHO_AM_I != m_buff[0]) ret = RET_ERROR;

        RESET();
        return ret;
    }


    RetType readReg(uint8_t reg, uint8_t *buff, size_t len) {
        RESUME();

        m_i2cAddr.mem_addr = reg;
        RetType ret = CALL(m_i2c->read(m_i2cAddr, buff, len));

        RESET();
        return ret;
    }

    RetType readReg(uint8_t reg, uint8_t *buff, size_t len, uint8_t mask, uint32_t timeout = 0) {
        RESUME();

        m_i2cAddr.mem_addr = reg;
        RetType ret = CALL(m_i2c->read(m_i2cAddr, buff, len, timeout));
        *buff &= mask;

        RESET();
        return ret;
    }

    RetType writeReg(uint8_t reg, uint8_t newVal, size_t len, uint8_t mask) {
        RESUME();

        m_i2cAddr.mem_addr = reg;
        RetType ret = CALL(m_i2c->read(m_i2cAddr, m_buff, len));
        if (RET_SUCCESS == ret) {
            m_buff[0] &= ~mask;
            m_buff[0] |= newVal;

            ret = CALL(m_i2c->write(m_i2cAddr, m_buff, len));
        }

        RESET();
        return ret;
    }
};

#endif //LAUNCH_CORE_LSM6DSL_H
