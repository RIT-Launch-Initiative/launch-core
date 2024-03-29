/**
 * Platform Independent LSM6DSL IMU Driver Using scheduler
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LSM6DSL_H
#define LAUNCH_CORE_LSM6DSL_H


#define LSM6DSL_DATA_STRUCT(variable_name) LSM6DSL_DATA_T variable_name = {.id = 11000, .x_accel = 0, .x_gyro = 0, .y_accel = 0, .y_gyro = 0, .z_accel = 0, .z_gyro = 0}

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

    explicit LSM6DSL(I2CDevice &i2CDevice, uint16_t address = LSM6DSL_I2C_ADDR_SECONDARY, const char *name = "LSM6DSL") : Device(name), m_i2c(&i2CDevice),
    m_i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

    RetType init() override {
        RESUME();
        m_i2cAddr.mem_addr = LSM6DSL_ACC_GYRO_WHO_AM_I_REG;

        RetType ret = CALL(checkChipID());
        ERROR_CHECK(ret);

        // Enable reg addr automatically incremented during multi byte access with serial intf
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, LSM6DSL_ACC_GYRO_IF_INC_ENABLED, LSM6DSL_ACC_GYRO_IF_INC_MASK));
        ERROR_CHECK(ret);

        // Enable BDU
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL3_C, LSM6DSL_ACC_GYRO_BDU_BLOCK_UPDATE, LSM6DSL_ACC_GYRO_BDU_MASK));
        ERROR_CHECK(ret);

        // FIFO Mode Select
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_FIFO_CTRL5, LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS, LSM6DSL_ACC_GYRO_FIFO_MODE_MASK));
        ERROR_CHECK(ret);

        // ODR Selection
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_ACC_GYRO_ODR_G_104Hz, LSM6DSL_ACC_GYRO_ODR_G_MASK));
        ERROR_CHECK(ret);

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_GYRO_ODR_XL_104Hz, LSM6DSL_ACC_GYRO_ODR_XL_MASK));
        ERROR_CHECK(ret);

        // Full Scale
        ret = CALL(setAccelFullScale(LSM6DSL_ACC_GYRO_FS_XL_16g));
        ERROR_CHECK(ret);

        ret = CALL(setGyroFullScale(LSM6DSL_ACC_GYRO_FS_G_2000dps));
        ERROR_CHECK(ret);

        ret = CALL(calibrate());

        RESET();
        return ret;
    }


    /**
     * Get all sensor data
     * @param data - Pointer to Struct to LSM6DSL Data
     * @return Scheduler Status
     */
    RetType getData(LSM6DSL_DATA_T *data) {
        RESUME();

        RetType ret = CALL(getData(&data->x_accel, &data->y_accel, &data->z_accel, &data->x_gyro, &data->y_gyro, &data->z_gyro));

        data->x_accel -= m_accelBias[0];
        data->y_accel -= m_accelBias[1];
        data->z_accel -= m_accelBias[2];

        data->x_gyro -= m_gyroBias[0];
        data->y_gyro -= m_gyroBias[1];
        data->z_gyro -= m_gyroBias[2];

        RESET();
        return ret;
    }

    /**
     * Retrieve all data at once and calculate the results
     * @param accelX - pointer to X accel data
     * @param accelY - pointer to Y accel data
     * @param accelZ - pointer to Z accel data
     * @param gyroX - pointer to X gyro data
     * @param gyroY - pointer to Y gyro data
     * @param gyroZ - pointer to Z gyro data
     * @return Scheduler Status
     */
    RetType getData(int32_t *accelX, int32_t *accelY, int32_t *accelZ, int32_t *gyroX, int32_t *gyroY, int32_t *gyroZ) {
        RESUME();

        RetType ret = CALL(getAxesRaw(m_buff));
        ERROR_CHECK(ret);

        ret = CALL(calculateGyroAxes(&m_buff[0], gyroX, gyroY, gyroZ));
        ERROR_CHECK(ret);

        ret = CALL(calculateAccelAxes(&m_buff[6], accelX, accelY, accelZ));

        RESET();
        return ret;
    }

    /**
     * Get all raw sensor data
     * @param buff - pointer to buffer of at least 12 bytes
     * @return Scheduler Status
     */
    RetType getAxesRaw(uint8_t *buff) {
        RESUME();

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_OUTX_L_G, buff, 12));

        RESET();
        return ret;
    }

    /**********************************************************
     * Acceleration Functions
     **********************************************************/

    /**
     * Get acceleration data in m/s^2
     * @param accelX - Pointer to X Acceleration data
     * @param accelY - Pointer to X Acceleration data
     * @param accelZ - Pointer to X Acceleration data
     * @return Scheduler Status
     */
    RetType getAccelAxes(int32_t *accelX, int32_t *accelY, int32_t *accelZ) {
        RESUME();

        RetType ret = CALL(getAccelAxesRaw(m_buff));
        if (RET_SUCCESS == ret) {
            ret = CALL(calculateAccelAxes(m_buff, accelX, accelY, accelZ));
        }

        RESET();
        return ret;
    }


    /**
     * Calculate accelerometer values
     * @param buff - Pointer to buffer of raw accel data containing 6 bytes
     * @param accelX - Pointer to X axis accelerometer value
     * @param accelY - Pointer to Y axis accelerometer value
     * @param accelZ - Pointer to Z axis accelerometer value
     * @return Scheduler Status
     */
    RetType calculateAccelAxes(uint8_t *buff, int32_t *accelX, int32_t *accelY, int32_t *accelZ) {
        RESUME();

        RetType ret = CALL(getAccelSens(&m_sens));
        if (ret == RET_SUCCESS) {
            float sens = determineAccelSens(m_sens);

            int16_t rawX = static_cast<int16_t>(buff[1] << 8) | static_cast<int16_t>(buff[0]);
            int16_t rawY = static_cast<int16_t>(buff[3] << 8) | static_cast<int16_t>(buff[2]);
            int16_t rawZ = static_cast<int16_t>(buff[5] << 8) | static_cast<int16_t>(buff[4]);

            *accelX = static_cast<int32_t>(rawX * sens);
            *accelY = static_cast<int32_t>(rawY * sens);
            *accelZ = static_cast<int32_t>(rawZ * sens);

            *accelX = (*accelX * GRAVITY) / 1000;
            *accelY = (*accelY * GRAVITY) / 1000;
            *accelZ = (*accelZ * GRAVITY) / 1000;
        }

        RESET();
        return ret;
    }

    /**
     * Get the raw acceleration data
     * @param buff - Pointer to buffer that can store at least 6 byts
     * @return Scheduler Status
     */
    RetType getAccelAxesRaw(uint8_t *buff) {
        RESUME();

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_OUTX_L_XL, buff, 6));

        RESET();
        return ret;
    }

    /**
     * Get the acceleration sensitivity
     * @param sens - Pointer to acceleration sensitivity
     * @return Scheduler Status
     */
    RetType getAccelSens(uint8_t *sens) {
        RESUME();

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_CTRL1_XL, sens, LSM6DSL_ACC_GYRO_FS_XL_MASK));

        RESET();
        return ret;
    }


    /**
     * Set the full scale acceleration range value
     * @param fullScale - Full Scale value to set
     * @return Scheduler Status
     */
    RetType setAccelFullScale(LSM6DSL_ACC_GYRO_FS_XL_t fullScale) {
        RESUME();

        m_buff[0] = fullScale;
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, m_buff[0], LSM6DSL_ACC_GYRO_FS_XL_MASK));

        RESET();
        return ret;
    }

    /**
     * Set the output data rate for acceleration
     * @param odr - Output Data Rate value to set
     * @return Scheduler Status
     */
    RetType setAccelODR(LSM6DSL_ACC_GYRO_ODR_XL_t odr) {
        RESUME();

        m_buff[0] = odr;
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, m_buff[0], LSM6DSL_ACC_GYRO_ODR_XL_MASK));

        RESET();
        return ret;
    }

    /**********************************************************
     * Gyroscope Functions
     **********************************************************/

    /**
     * Get gyroscope data in degrees per second
     * @param accelX - Pointer to X Gyroscope data
     * @param accelY - Pointer to Y Gyroscope data
     * @param accelZ - Pointer to Z Gyroscope data
     * @return Scheduler Status
     */
    RetType getGyroAxes(int32_t *gyroX, int32_t *gyroY, int32_t *gyroZ) {
        RESUME();

        RetType ret = CALL(getGyroAxesRaw(m_buff));
        if (RET_SUCCESS == ret) {
            ret = CALL(calculateGyroAxes(m_buff, gyroX, gyroY, gyroZ));
        }

        RESET();
        return ret;
    }

    /**
     * Get the raw gyroscope data
     * @param buff - Pointer to buffer that can store at least 6 bytes
     * @return Scheduler Status
     */
    RetType getGyroAxesRaw(uint8_t *buff) {
        RESUME();

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_OUTX_L_G, buff, 6));

        RESET();
        return ret;
    }

    /**
     * Calculate gyroscope values
     * @param buff - Pointer to raw gyroscope data containing 6 bytes
     * @param gyroX - Pointer to X axis gyroscope value
     * @param gyroY - Pointer to Y axis gyroscope value
     * @param gyroZ - Pointer to Z axis gyroscope value
     * @return Scheduler Status
     */
    RetType calculateGyroAxes(uint8_t *buff, int32_t *gyroX, int32_t *gyroY, int32_t *gyroZ) {
        RESUME();

        RetType ret = CALL(getGyroSens(&m_sens));
        if (RET_SUCCESS == ret) {
            int16_t rawX = static_cast<int16_t>(buff[1] << 8) | static_cast<int16_t>(buff[0]);
            int16_t rawY = static_cast<int16_t>(buff[3] << 8) | static_cast<int16_t>(buff[2]);
            int16_t rawZ = static_cast<int16_t>(buff[5] << 8) | static_cast<int16_t>(buff[4]);

            float sens = determineGyroSens(sens);

            *gyroX = static_cast<int32_t>(rawX * sens);
            *gyroY = static_cast<int32_t>(rawY * sens);
            *gyroZ = static_cast<int32_t>(rawZ * sens);
        }

        RESET();
        return ret;
    }

    /**
     * Get gyroscope sensitivity
     * @param sens - Pointer to sensitivity value
     * @return Scheduler Status
     */
    RetType getGyroSens(uint8_t *sens) {
        RESUME();

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_CTRL2_G, sens, 1, LSM6DSL_ACC_GYRO_FS_125_MASK));

        if (RET_SUCCESS == ret && LSM6DSL_ACC_GYRO_FS_125_ENABLED != *sens) {
            ret = CALL(readReg(LSM6DSL_ACC_GYRO_CTRL2_G, sens, 1, LSM6DSL_ACC_GYRO_FS_G_MASK));
        } else {
            *sens = 0xFF; // Will hit default case when checked
        }

        RESET();
        return ret;
    }

    /**
     * Set the gyroscope sensitivity
     * @param sens - Pointer to gyroscope sensitivity
     * @return Scheduler Status
     */
    RetType setGyroFullScale(LSM6DSL_ACC_GYRO_FS_G_t fullScale) {
        RESUME();

        RetType ret;
        if (LSM6DSL_ACC_GYRO_FS_G_125dps == fullScale) {
            ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_ACC_GYRO_FS_125_ENABLED, LSM6DSL_ACC_GYRO_FS_125_MASK));
        } else {
            ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, LSM6DSL_ACC_GYRO_FS_125_DISABLED, LSM6DSL_ACC_GYRO_FS_125_MASK));

            if (RET_SUCCESS == ret) {
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, fullScale, LSM6DSL_ACC_GYRO_FS_G_MASK));
            }
        }

        RESET();
        return ret;
    }

    /**
     * Set the gyroscope output data rate
     * @param sens - Output data rate to set
     * @return Scheduler Status
     */
    RetType setGyroODR(LSM6DSL_ACC_GYRO_ODR_G_t odr) {
        RESUME();

        m_buff[0] = odr;
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL2_G, m_buff[0], LSM6DSL_ACC_GYRO_ODR_G_MASK));

        RESET();
        return ret;
    }

    /********************************************************************
     * Free Fall Settings
     ********************************************************************/

    /**
     * Enable sending an interrupt when free fall is detected
     * @param interruptPin - Pin to send the interrupt to
     * @return Scheduler Status
     */
    RetType enableFreeFallDetection(LSM6DSL_Interrupt_Pin_t interruptPin) {
        RESUME();

        RetType ret = CALL(setAccelODR(LSM6DSL_ACC_GYRO_ODR_XL_416Hz));
        ERROR_CHECK(ret);

        // Full Scale Select
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_GYRO_FS_XL_2g, LSM6DSL_ACC_GYRO_FS_XL_MASK));
        ERROR_CHECK(ret);

        // FF Duration
        uint8_t duration = 0x06;
        uint8_t lowVal = duration & 0x1F;
        lowVal = lowVal << LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_POSITION;
        lowVal &= LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_FREE_FALL, lowVal, LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK));
        ERROR_CHECK(ret);

        uint8_t highVal = (duration >> 5) & 0x1;
        highVal = highVal << LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_POSITION;
        highVal &= LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, highVal, LSM6DSL_ACC_GYRO_FS_XL_MASK));
        ERROR_CHECK(ret);

        // Wake Duration
        duration = 0x00 << LSM6DSL_ACC_GYRO_WAKE_DUR_POSITION;
        duration &= LSM6DSL_ACC_GYRO_WAKE_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, duration, LSM6DSL_ACC_GYRO_WAKE_DUR_MASK));
        ERROR_CHECK(ret);

        // Timer HR
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, LSM6DSL_ACC_GYRO_TIMER_HR_6_4ms, LSM6DSL_ACC_GYRO_TIMER_HR_MASK));
        ERROR_CHECK(ret);

        // Sleep Duration
        duration = 0x00 << LSM6DSL_ACC_GYRO_SLEEP_DUR_POSITION;
        duration &= LSM6DSL_ACC_GYRO_SLEEP_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, duration, LSM6DSL_ACC_GYRO_SLEEP_DUR_MASK));
        ERROR_CHECK(ret);

        // Free Fall THS
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_GYRO_FF_THS_312mg, LSM6DSL_ACC_GYRO_FF_THS_MASK));
        ERROR_CHECK(ret);

        // Enable Interrupts
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_CFG1, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED, LSM6DSL_ACC_GYRO_INT_EN_MASK));
        ERROR_CHECK(ret);

        /* Enable free fall event on either INT1 or INT2 pin */
        switch (interruptPin) {
            case LSM6DSL_INT1_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_FF_ENABLED, LSM6DSL_ACC_GYRO_INT1_FF_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }

            case LSM6DSL_INT2_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT1_FF_ENABLED, LSM6DSL_ACC_GYRO_INT2_FF_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }

            default:
                RESET();
                return RET_ERROR;
        }

        RESET();
        return RET_SUCCESS;
    }

    /**
    * Disable sending an interrupt when free fall is detected
    * @param interruptPin - Pin to disable sending the interrupt to
    * @return Scheduler Status
    */
    RetType disableFreeFallDetection() {
        RESUME();

        // Disable FF events on pins
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_FF_DISABLED, LSM6DSL_ACC_GYRO_INT1_FF_MASK));
        ERROR_CHECK(ret);

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT1_FF_DISABLED, LSM6DSL_ACC_GYRO_INT2_FF_MASK));
        ERROR_CHECK(ret);

        // Disable Interrupts
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_CFG1, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED, LSM6DSL_ACC_GYRO_INT_EN_MASK));
        ERROR_CHECK(ret);

        // Set Free Fall Duration
        uint8_t duration = 0x00;

        uint8_t lowVal = duration & 0x1F;
        lowVal = lowVal << LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_POSITION;
        lowVal &= LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_FREE_FALL, lowVal, LSM6DSL_ACC_GYRO_FF_FREE_FALL_DUR_MASK));
        ERROR_CHECK(ret);

        uint8_t highVal = (duration >> 5) & 0x1;
        highVal = highVal << LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_POSITION;
        highVal &= LSM6DSL_ACC_GYRO_FF_WAKE_UP_DUR_MASK;

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_WAKE_UP_DUR, highVal, LSM6DSL_ACC_GYRO_FS_XL_MASK));
        ERROR_CHECK(ret);

        // Set Free Fall THS Setting
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL1_XL, LSM6DSL_ACC_GYRO_FF_THS_156mg, LSM6DSL_ACC_GYRO_FF_THS_MASK));

        RESET();
        return ret;
    }

    /**
     * Set the threshold for freefall interrupts
     * @param threshold - Threshold value to set
     * @return Scheduler Status
     */
    RetType setFreeFallThreshold(uint8_t threshold) {
        RESUME();

        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_FREE_FALL, threshold, LSM6DSL_ACC_GYRO_FF_THS_MASK));

        RESET();
        return ret;
    }

    /********************************************************************
     * Tilt Detection Settings
     ********************************************************************/

    /**
    * Enable sending an interrupt when tilt is detected
    * @param interruptPin - Pin to send the interrupt to
    * @return Scheduler Status
    */
    RetType enableTiltDetection(LSM6DSL_Interrupt_Pin_t interruptPin) {
        RESUME();

        // Output Data Rate
        RetType ret = CALL(setGyroODR(LSM6DSL_ACC_GYRO_ODR_G_26Hz));
        ERROR_CHECK(ret);

        // Full Scale Selection
        ret = CALL(setGyroFullScale(LSM6DSL_ACC_GYRO_FS_G_125dps));
        ERROR_CHECK(ret);

        // Enable Embedded Functionalities
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL10_C, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED, LSM6DSL_ACC_GYRO_FUNC_EN_MASK));
        ERROR_CHECK(ret);

        // Enable Tilt Calculation
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL10_C, LSM6DSL_ACC_GYRO_TILT_ENABLED, LSM6DSL_ACC_GYRO_TILT_MASK));
        ERROR_CHECK(ret);

        // Enable Tilt Detection
        switch (interruptPin) {
            case LSM6DSL_INT1_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_TILT_ENABLED, LSM6DSL_ACC_GYRO_INT1_TILT_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }
                break;

            case LSM6DSL_INT2_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT2_TILT_ENABLED, LSM6DSL_ACC_GYRO_INT2_TILT_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }
                break;

            default:
                ret = RET_ERROR;
        }

        RESET();
        return ret;
    }

    /**
    * Disable sending an interrupt when tilt is detected
    * @param interruptPin - Pin to disable sending the interrupt to
    * @return Scheduler Status
    */
    RetType disableTiltDetection() {
        RESUME();

        // Disable Tilt Events
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_TILT_DISABLED, LSM6DSL_ACC_GYRO_INT1_TILT_MASK));
        ERROR_CHECK(ret);

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT2_TILT_DISABLED, LSM6DSL_ACC_GYRO_INT2_TILT_MASK));
        ERROR_CHECK(ret);

        // Disable Tilt Calculations
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL10_C, LSM6DSL_ACC_GYRO_TILT_DISABLED, LSM6DSL_ACC_GYRO_TILT_MASK));
        ERROR_CHECK(ret);

        // Disable Embedded Functionalities
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_CTRL10_C, LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED, LSM6DSL_ACC_GYRO_FUNC_EN_MASK));

        RESET();
        return ret;
    }

    /********************************************************************
     * Six Degree Orientation Settings
     ********************************************************************/

    /**
    * Enable sending an interrupt when 6DoF is detected
    * @param interruptPin - Pin to send the interrupt to
    * @return Scheduler Status
    */
    RetType enable6DOrientation(LSM6DSL_Interrupt_Pin_t interruptPin) {
        RESUME();

        // Output Data Rate selection
        RetType ret = CALL(setAccelODR(LSM6DSL_ACC_GYRO_ODR_XL_416Hz));
        ERROR_CHECK(ret);

        ret = CALL(setAccelFullScale(LSM6DSL_ACC_GYRO_FS_XL_2g));
        ERROR_CHECK(ret);

        // Set 6D Threshold
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, LSM6DSL_ACC_GYRO_SIXD_THS_60_degree, LSM6DSL_ACC_GYRO_SIXD_THS_MASK));
        ERROR_CHECK(ret);

        /* Enable basic Interrupts */
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_CFG1, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED, LSM6DSL_ACC_GYRO_INT_EN_MASK));
        ERROR_CHECK(ret);

        // Enable 6D orientation on either INT1 or INT2 pin
        switch (interruptPin) {
            case LSM6DSL_INT1_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_6D_ENABLED, LSM6DSL_ACC_GYRO_INT1_6D_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }

                break;
            case LSM6DSL_INT2_PIN:
                ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT2_6D_ENABLED, LSM6DSL_ACC_GYRO_INT2_6D_MASK));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }

                break;
            default:
                RESET();
                return RET_ERROR;
        }

        RESET();
        return RET_SUCCESS;
    }

    /**
    * Disable sending an interrupt when 6DoF is detected
    * @param interruptPin - Pin to disable sending the interrupt to
    * @return Scheduler Status
    */
    RetType disableSixDoF() {
        RESUME();

        // Disable 6D Interrupts
        RetType ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD1_CFG, LSM6DSL_ACC_GYRO_INT1_6D_DISABLED, LSM6DSL_ACC_GYRO_INT1_6D_MASK));
        ERROR_CHECK(ret);

        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_MD2_CFG, LSM6DSL_ACC_GYRO_INT2_6D_DISABLED, LSM6DSL_ACC_GYRO_INT2_6D_MASK));
        ERROR_CHECK(ret);

        // Disable Basic Interrupts
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_CFG1, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED, LSM6DSL_ACC_GYRO_INT_EN_MASK));
        ERROR_CHECK(ret);

        /* Reset 6D threshold. */
        ret = CALL(writeReg(LSM6DSL_ACC_GYRO_TAP_THS_6D, LSM6DSL_ACC_GYRO_SIXD_THS_80_degree, LSM6DSL_ACC_GYRO_SIXD_THS_MASK));

        RESET();
        return ret;
    }

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
    // Variables for handling sensor calibration
    uint8_t m_samplesTaken = 0;
    uint8_t m_calibErrCnt = 0;
    int32_t m_calibBuff[6] = {};
    int32_t m_accelBias[3] = {};
    int32_t m_gyroBias[3] = {};
    static constexpr int NUM_CALIBRATION_SAMPLES = 50;

    I2CDevice *m_i2c;
    I2CAddr_t m_i2cAddr;
    uint8_t m_buff[12] = {};
    uint8_t m_sens = 0;



    /**
     * Check the chip ID to ensure valid communication
     * @return Result of chip ID check
     */
    RetType checkChipID() {
        RESUME();

        RetType ret = CALL(readReg(LSM6DSL_ACC_GYRO_WHO_AM_I_REG, m_buff, 1, LSM6DSL_ACC_GYRO_WHO_AM_I_BIT_MASK, 50));
        if (RET_SUCCESS == ret && LSM6DSL_ACC_GYRO_WHO_AM_I != m_buff[0]) ret = RET_ERROR;

        RESET();
        return ret;
    }


    /**
     * Calibrate the sensor
     * @return Scheduler Status
     */
    RetType calibrate() {
        RESUME();

        while (m_samplesTaken < NUM_CALIBRATION_SAMPLES) {
            RetType ret = CALL(getData(&m_calibBuff[0], &m_calibBuff[1], &m_calibBuff[2], &m_calibBuff[3], &m_calibBuff[4], &m_calibBuff[5]));
            if (RET_SUCCESS == ret) {
                m_accelBias[0] += m_calibBuff[0];
                m_accelBias[1] += m_calibBuff[1];
                m_accelBias[2] += m_calibBuff[2];
                m_gyroBias[0] += m_calibBuff[3];
                m_gyroBias[1] += m_calibBuff[4];
                m_gyroBias[2] += m_calibBuff[5];
                ++m_samplesTaken;
            } else {
                if (++m_calibErrCnt >= 3) { // 3 strikes you're out
                    RESET();
                    return RET_ERROR;
                }
            }
        }

        for (int i = 0; i < 3; i++) {
            m_accelBias[i] /= NUM_CALIBRATION_SAMPLES;
            m_gyroBias[i] /= NUM_CALIBRATION_SAMPLES;
        }

        RESET();
        return RET_SUCCESS;
    }

    /**
     * Helper for reading register(s)
     * @param reg - Register to start reading from
     * @param buff - Buffer for register values
     * @param len - Bytes to read into the buffer
     * @return Scheduler Status
     */
    RetType readReg(uint8_t reg, uint8_t *buff, size_t len) {
        RESUME();

        m_i2cAddr.mem_addr = reg;
        RetType ret = CALL(m_i2c->read(m_i2cAddr, buff, len));

        RESET();
        return ret;
    }

    /**
     * Helper for reading register(s) with masking
     * @param reg - Register to start reading from
     * @param buff - Buffer for register values
     * @param len - Bytes to read into the buffer
     * @param mask - Bit Mask
     * @param timeout - Time that transaction needs to finish before
     * @return Scheduler Status
     */
    RetType readReg(uint8_t reg, uint8_t *buff, size_t len, uint8_t mask, uint32_t timeout = 0) {
        RESUME();

        m_i2cAddr.mem_addr = reg;
        RetType ret = CALL(m_i2c->read(m_i2cAddr, buff, len, timeout));
        *buff &= mask;

        RESET();
        return ret;
    }

    /**
     * Write to a register
     * @param reg - Register value
     * @param val - Value to write
     * @param mask - Bit Mask
     * @return Scheduler Status
     */
    RetType writeReg(uint8_t reg, uint8_t val, uint8_t mask) {
        RESUME();

        m_i2cAddr.mem_addr = reg;
        RetType ret = CALL(m_i2c->read(m_i2cAddr, m_buff, 1));
        if (RET_SUCCESS == ret) {
            m_buff[0] &= ~mask;
            m_buff[0] |= val;

            ret = CALL(m_i2c->write(m_i2cAddr, m_buff, 1));
        }

        RESET();
        return ret;
    }

    /**
     * Determine accelerometer sensitivity value based on full scale
     * @param fullScaleValue - Current full scale value
     * @return Accelerometer Sensitivity
     */
    float determineAccelSens(uint8_t fullScaleValue) {
        switch (fullScaleValue) {
            case LSM6DSL_ACC_GYRO_FS_XL_2g:
                return LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G;
            case LSM6DSL_ACC_GYRO_FS_XL_4g:
                return LSM6DSL_ACC_SENSITIVITY_FOR_FS_4G;
            case LSM6DSL_ACC_GYRO_FS_XL_8g:
                return LSM6DSL_ACC_SENSITIVITY_FOR_FS_8G;
            case LSM6DSL_ACC_GYRO_FS_XL_16g:
                return LSM6DSL_ACC_SENSITIVITY_FOR_FS_16G;
            default:
                return LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G;
        }
    }

    /**
     * Determine gyroscope sensitivity value based on full scale
     * @param fullScaleValue - Current full scale value
     * @return Gyroscope Sensitivity
     */
    float determineGyroSens(uint8_t fullScaleValue) {
        switch (fullScaleValue) {
            case LSM6DSL_ACC_GYRO_FS_G_245dps:
                return LSM6DSL_GYRO_SENSITIVITY_FOR_FS_245DPS;
            case LSM6DSL_ACC_GYRO_FS_G_500dps:
                return LSM6DSL_GYRO_SENSITIVITY_FOR_FS_500DPS;
            case LSM6DSL_ACC_GYRO_FS_G_1000dps:
                return LSM6DSL_GYRO_SENSITIVITY_FOR_FS_1000DPS;
            case LSM6DSL_ACC_GYRO_FS_G_2000dps:
                return LSM6DSL_GYRO_SENSITIVITY_FOR_FS_2000DPS;
            default:
                return LSM6DSL_GYRO_SENSITIVITY_FOR_FS_125DPS;
        }
    }
};

#endif //LAUNCH_CORE_LSM6DSL_H
