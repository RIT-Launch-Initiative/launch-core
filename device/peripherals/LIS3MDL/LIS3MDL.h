/**
 * LIS3MDL Platform Independent Driver
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LIS3MDL_H
#define LAUNCH_CORE_LIS3MDL_H
#define LIS3MDL_DATA_STRUCT(variable_name) LIS3MDL_DATA_T variable_name = {.id = 15000, .x_mag = 0, .y_mag = 0, .z_mag = 0, .temperature = 0}

#include "lis3mdl_reg.h"
#include "device/I2CDevice.h"
#include "sched/macros.h"


using LIS3MDL_DATA_T = struct {
    const uint16_t id;
    float x_mag;
    float y_mag;
    float z_mag;
    float temperature;
};

class LIS3MDL : public Device {
public:
    typedef enum {
        LIS3MDL_I2C_ADDR_PRIMARY = 0x1C,
        LIS3MDL_I2C_ADDR_SECONDARY = 0x1E,
    } LIS3MDL_I2C_ADDR;

    explicit LIS3MDL(I2CDevice &i2cDevice, const uint16_t address = LIS3MDL::LIS3MDL_I2C_ADDR_PRIMARY, const char *name = "LIS3MDL")
            : Device(name), mI2C(&i2cDevice),
              i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

    RetType init() override {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_WHO_AM_I, m_buff, 1, 50));
        if (LIS3MDL_ID != m_buff[0]) ret = RET_ERROR;

        if (RET_SUCCESS == ret) {
            ret = CALL(initSettings());
        }

        RESET();
        return ret;
    }

    RetType getData(LIS3MDL_DATA_T *data) {
        RESUME();

        RetType ret = CALL(pullSensorData(&data->x_mag, &data->y_mag, &data->z_mag, &data->temperature));

        RESET();
        return ret;
    }

    /**
     * Gets the calculated sensor data
     *
     * @param magX - X Gauss
     * @param magY - Y Gauss
     * @param magZ - Z Gauss
     * @param temp - Celsius Temperature
     * @return
     */
    RetType pullSensorData(float *magX, float *magY, float *magZ, float *temp) {
        RESUME();

        RetType ret = CALL(getRawMagnetic(m_buff));
        if (RET_SUCCESS == ret) {
            *magX = fs16ToGauss((m_buff[1] << 8) | (int16_t) m_buff[0]);
            *magY = fs16ToGauss((m_buff[3] << 8) | (int16_t) m_buff[2]);
            *magZ = fs16ToGauss((m_buff[5] << 8) | (int16_t) m_buff[4]);
        }

        ret = CALL(getRawTemp(m_buff));
        if (RET_SUCCESS == ret) {
            *temp = lsbToCelsius((m_buff[1] << 8) + (int16_t) m_buff[0]);
        }

        RESET();
        return ret;
    }

    RetType getRawMagnetic(uint8_t *val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_OUT_X_L, m_buff, 6));

        RESET();
        return ret;
    }

    RetType getRawTemp(uint8_t *val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_TEMP_OUT_L, m_buff, 2));

        RESET();
        return ret;
    }

    RetType readReg(uint8_t reg, uint8_t *data, uint16_t len, uint32_t timeout = 0) {
        RESUME();

        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C->read(i2cAddr, data, len, timeout));

        RESET();
        return ret;
    };

    RetType writeReg(uint8_t reg, uint8_t *data, uint16_t len, uint32_t timeout = 0) {
        RESUME();

        i2cAddr.mem_addr = reg;

        RetType ret = CALL(mI2C->write(i2cAddr, data, len, timeout));

        RESET();
        return ret;
    };

    /**
     * Sets the output data rate
     * @param val Modify the value of the OM field in the CTRL_REG1 register
     * @return
     */
    RetType setDataRate(lis3mdl_om_t val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, (uint8_t * ) &mCtrlReg1, 1));
        ERROR_CHECK(ret);

        mCtrlReg1.om = val;
        ret = CALL(readReg(LIS3MDL_CTRL_REG4, (uint8_t * ) &mCtrlReg4, 1));
        ERROR_CHECK(ret);

        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, (uint8_t * ) &mCtrlReg1, 1));
        ERROR_CHECK(ret);

        mCtrlReg4.omz = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG4, (uint8_t * ) &mCtrlReg4, 1));

        RESET();
        return ret;
    }

    /**
     * Enables or disables temperature measurements
     * @param val Value of temperature measurement enable bit in CTRL_REG1 register
     * @return
     */
    RetType setTempMeas(uint8_t val) {
        RESUME();


        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, reinterpret_cast<uint8_t*>(&mCtrlReg1), 1));
        ERROR_CHECK(ret);

        mCtrlReg1.temp_en = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, reinterpret_cast<uint8_t*>(&mCtrlReg1), 1));

        RESET();
        return ret;
    }

    /**
     * Sets the full scale of the magnetometer
     *
     * @param val Modify the value of the FS field in the CTRL_REG2 register
     * @return
     */
    RetType setFullScale(uint8_t val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&mCtrlReg2), 1));
        ERROR_CHECK(ret);
        if (LIS3MDL_4_GAUSS == val || LIS3MDL_8_GAUSS == val || LIS3MDL_12_GAUSS == val || LIS3MDL_16_GAUSS == val) {
            mCtrlReg2.fs = val;
        }

        ret = CALL(writeReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&mCtrlReg2), 1));

        RESET();
        return ret;
    }

    /**
     * Sets the operating mode of the magnetometer
     * @param val The value of the MD field in the CTRL_REG3 register
     * @return
     */
    RetType setOperatingMode(lis3mdl_md_t val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&mCtrlReg3), 1));

        if (RET_SUCCESS == ret) {
            mCtrlReg3.md = val;
            ret = CALL(writeReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&mCtrlReg3), 1));
        }

        RESET();
        return ret;
    }

    /**
     * Sets the power mode of the magnetometer
     * @param val Value of the LP field in the CTRL_REG3 register
     *            1 = low power mode   0 = normal mode
     * @return
     */
    RetType setPowerMode(uint8_t val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&mCtrlReg3), 1));

        if (RET_SUCCESS == ret) {
            mCtrlReg3.lp = val;
            ret = CALL(writeReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&mCtrlReg3), 1));
        }

        RESET();
        return ret;
    }

    RetType setBlockDataUpdate(uint8_t val) {
        RESUME();


        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&mCtrlReg5), 1));
        if (RET_SUCCESS == ret) {
            mCtrlReg5.bdu = val;
            ret = CALL(writeReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&mCtrlReg5), 1));
        }

        RESET();
        return ret;
    }


    RetType setHighPartCycle(uint8_t val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_CTRL_REG5, &val, 1));

        if (val == 0 && ret == RET_SUCCESS) {
            ret = CALL(readReg(LIS3MDL_CTRL_REG5, reinterpret_cast<uint8_t *>(&mCtrlReg5), 1));
        }

        RESET();
        return ret;
    }


    /**
     * Software reset. Restore default values in user registers
     *
     * @param val
     * @return
     */
    RetType reset() {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&mCtrlReg2), 1));

        RESET();
        return ret;
    }

    RetType reboot() {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&mCtrlReg2), 1));

        RESET();
        return ret;
    }

    /**
     * Set the endianness of the data output
     *
     * @param val 1 = little endian   0 = big endian
     * @return
     */
    RetType setDataEndianness(uint8_t val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&mCtrlReg4), 1));

        RESET();
        return ret;
    }

    /**
     * Get the endianness of the data output
     *
     * @param val 1 = little endian   0 = big endian
     * @return
     */
    RetType getDataEndianness(uint8_t *val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&mCtrlReg4), 1));

        *val = mCtrlReg4.ble;

        RESET();
        return ret;
    }

    RetType setConfig(lis3mdl_int_cfg_t *val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_INT_CFG, reinterpret_cast<uint8_t *>(val), 1));

        RESET();
        return ret;
    }

    /**
     * Enable INT pin
     * @param val 1 = enable   0 = disable
     * @return
     */
    RetType enableInterrupt(uint8_t val) {
        RESUME();

        RetType ret = RET_ERROR;
        if (LIS3MDL_ACTIVE_HIGH == val || LIS3MDL_ACTIVE_LOW == val) {
            ret = CALL(writeReg(LIS3MDL_INT_CFG, &val, 1));
        }

        RESET();
        return ret;
    }

    RetType setInterruptNotifications(uint8_t val) {
        RESUME();

        RetType ret = RET_ERROR;
        if (LIS3MDL_INT_LATCHED == val || LIS3MDL_INT_PULSED == val) {
            ret = CALL(writeReg(LIS3MDL_INT_SRC, &val, 1));
        }

        RESET();
        return ret;
    }

    /**
     * Set the polarity of the INT pin
     *
     * @param val 1 = active high   0 = active low
     * @return
     */
    RetType setInterruptPolarity(uint8_t val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_INT_CFG, &val, 1));

        RESET();
        return ret;
    }


    RetType getStatus(lis3mdl_status_reg_t *val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_STATUS_REG, reinterpret_cast<uint8_t *>(val), 1));

        RESET();
        return ret;
    }

private:
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;
    uint8_t m_buff[10];
    int16_t m_buff16[5];
    lis3mdl_ctrl_reg1_t mCtrlReg1;
    lis3mdl_ctrl_reg2_t mCtrlReg2;
    lis3mdl_ctrl_reg3_t mCtrlReg3;
    lis3mdl_ctrl_reg4_t mCtrlReg4;
    lis3mdl_ctrl_reg5_t mCtrlReg5;


    RetType initSettings() {
        RESUME();

        // Reset the sensor
        RetType ret = CALL(reset());

        // Enable Block Update
        ret = CALL(setBlockDataUpdate(PROPERTY_ENABLE));
        ERROR_CHECK(ret);

        // Set ODR
        ret = CALL(setDataRate(LIS3MDL_HP_80Hz));
        ERROR_CHECK(ret);

        // Set FS
        ret = CALL(setFullScale(LIS3MDL_16_GAUSS));
        ERROR_CHECK(ret);

        // Enable temp sensing
        ret = CALL(setTempMeas(PROPERTY_ENABLE));
        ERROR_CHECK(ret);

        // Set to continuous mode
        ret = CALL(setOperatingMode(LIS3MDL_CONTINUOUS_MODE));

        RESET();
        return ret;
    }

    float fs4ToGauss(int16_t lsb) {
        return lsb / 6842.0f;
    }

    float fs8ToGauss(int16_t lsb) {
        return lsb / 3421.0f;
    }

    float fs12ToGauss(int16_t lsb) {
        return lsb / 2281.0f;
    }

    float fs16ToGauss(int16_t lsb) {
        return lsb / 1711.0f;
    }

    float lsbToCelsius(int16_t lsb) {
        return (lsb / 8.0f) + 25.0f;
    }


};


#endif //LAUNCH_CORE_LIS3MDL_H


