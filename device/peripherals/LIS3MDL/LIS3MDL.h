/**
 * LIS3MDL sensor driver integrated with launch-core scheduler
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LIS3MDL_H
#define LAUNCH_CORE_LIS3MDL_H
#define LIS3MDL_DATA_STRUCT(variable_name) LIS3MDL_DATA_T variable_name = {.id = 15000, .x_mag = 0, .y_mag = 0, .z_mag = 0, .temperature = 0}

#include "device/I2CDevice.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "lis3mdl_reg.h"
#include "sched/macros/call.h"


using LIS3MDL_DATA_T = struct {
    const uint16_t id;
    float x_mag;
    float y_mag;
    float z_mag;
    float temperature;
};

enum LIS3MDL_I2C_ADDR {
    LIS3MDL_I2C_ADDR_PRIMARY = 0x1C,
    LIS3MDL_I2C_ADDR_SECONDARY = 0x1E,
};

class LIS3MDL : public Device {
public:
    LIS3MDL(I2CDevice &i2cDevice, const uint16_t address = LIS3MDL_I2C_ADDR_PRIMARY, const char *name = "LIS3MDL") : Device(name), mI2C(&i2cDevice), i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}
    RetType init() override {
        RESUME();

        static uint8_t whoAmI = 0;
        RetType ret = CALL(readReg(LIS3MDL_WHO_AM_I, &whoAmI, 1, 50));
        if (LIS3MDL_ID != whoAmI) ret = RET_ERROR;
        ERROR_CHECK(ret);

        ret = CALL(initSettings());
        
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

        static int16_t rawMagneticData[3];
        static int16_t rawTemp;

        RetType ret = CALL(getRawMagnetic(rawMagneticData));
        ERROR_CHECK(ret);

        *magX = fs16ToGauss(rawMagneticData[0]);
        *magY = fs16ToGauss(rawMagneticData[1]);
        *magZ = fs16ToGauss(rawMagneticData[2]);

        ret = CALL(getRawTemp(&rawTemp));
        *temp = lsbToCelsius(rawTemp);

        RESET();
        return ret;
    }

    RetType getRawMagnetic(int16_t *val) {
        RESUME();

        static uint8_t buff[6];
        RetType ret = CALL(readReg(LIS3MDL_OUT_X_L, buff, 6));
        ERROR_CHECK(ret);

        // TODO: Below calculations cause a hardfault
        val[0] = (int16_t) buff[1];
        val[0] = (val[0] * 256) + (int16_t) buff[0];
        val[1] = (int16_t) buff[3];
        val[1] = (val[1] * 256) + (int16_t) buff[2];
        val[2] = (int16_t) buff[5];
        val[2] = (val[2] * 256) + (int16_t) buff[4];

        RESET();
        return RET_SUCCESS;
    }

    RetType getRawTemp(int16_t *val) {
        RESUME();

        static uint8_t data[2];
        RetType ret = CALL(readReg(LIS3MDL_TEMP_OUT_L, (uint8_t *) data, 2));
        ERROR_CHECK(ret);

        *val = (int16_t) data[1];
        *val = (*val * 256) + (int16_t) data[0];

        RESET();
        return RET_SUCCESS;
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

    /**
     * Sets the output data rate
     * @param val Modify the value of the OM field in the CTRL_REG1 register
     * @return
     */
    RetType setDataRate(lis3mdl_om_t val) {
        RESUME();

        static lis3mdl_ctrl_reg1_t ctrlReg1;
        static lis3mdl_ctrl_reg4_t ctrlReg4;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, (uint8_t *) &ctrlReg1, 1));
        ERROR_CHECK(ret);

        ctrlReg1.om = val;
        ret = CALL(readReg(LIS3MDL_CTRL_REG4, (uint8_t *) &ctrlReg4, 1));
        ERROR_CHECK(ret);

        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, (uint8_t *) &ctrlReg1, 1));
        ERROR_CHECK(ret);

        ctrlReg4.omz = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG4, (uint8_t *) &ctrlReg4, 1));

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

        static lis3mdl_ctrl_reg1_t ctrlReg1;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, reinterpret_cast<uint8_t *>(&ctrlReg1), 1));
        ERROR_CHECK(ret);

        ctrlReg1.temp_en = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, reinterpret_cast<uint8_t *>(&ctrlReg1), 1));

        RESET();
        return ret;
    }

    /**
     * Sets the full scale of the magnetometer
     *
     * @param val Modify the value of the FS field in the CTRL_REG2 register
     * @return
     */
    RetType setFullScale(lis3mdl_fs_t val) {
        RESUME();

        static lis3mdl_ctrl_reg2_t ctrlReg2;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&ctrlReg2), 1));
        ERROR_CHECK(ret);

        ctrlReg2.fs = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&ctrlReg2), 1));

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

        static lis3mdl_ctrl_reg3_t ctrlReg3;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&ctrlReg3), 1));
        ERROR_CHECK(ret);

        ctrlReg3.md = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&ctrlReg3), 1));

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

        static lis3mdl_ctrl_reg3_t ctrlReg3;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&ctrlReg3), 1));
        ERROR_CHECK(ret);

        ctrlReg3.lp = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&ctrlReg3), 1));

        RESET();
        return ret;
    }

    RetType setBlockDataUpdate(uint8_t val) {
        RESUME();

        static lis3mdl_ctrl_reg5_t ctrlReg5;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&ctrlReg5), 1));
        ERROR_CHECK(ret);

        ctrlReg5.bdu = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&ctrlReg5), 1));

        RESET();
        return ret;
    }


    RetType setHighPartCycle(uint8_t val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_CTRL_REG5, &val, 1));

        if (val == 0 && ret == RET_SUCCESS) {
            static lis3mdl_ctrl_reg5_t ctrlReg5;
            ret = CALL(readReg(LIS3MDL_CTRL_REG5, reinterpret_cast<uint8_t *>(&ctrlReg5), 1));
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
    RetType reset(uint8_t val) {
        RESUME();
        static lis3mdl_ctrl_reg2_t ctrlReg2;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&ctrlReg2), 1));

        RESET();
        return ret;
    }

    RetType reboot(uint8_t val) {
        RESUME();
        static lis3mdl_ctrl_reg2_t ctrlReg2;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&ctrlReg2), 1));

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
        static lis3mdl_ctrl_reg4_t ctrlReg4;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&ctrlReg4), 1));

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
        static lis3mdl_ctrl_reg4_t ctrlReg4;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&ctrlReg4), 1));

        switch (ctrlReg4.ble) {
            case LIS3MDL_LSB_AT_LOW_ADD:
                *val = LIS3MDL_LSB_AT_LOW_ADD;
                break;

            case LIS3MDL_MSB_AT_LOW_ADD:
                *val = LIS3MDL_MSB_AT_LOW_ADD;
                break;

            default:
                *val = LIS3MDL_LSB_AT_LOW_ADD;
                break;
        }

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

        RetType ret = CALL(writeReg(LIS3MDL_INT_CFG, &val, 1));

        RESET();
        return ret;
    }

    RetType setInterruptNotifications(lis3mdl_lir_t *val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_INT_SRC, reinterpret_cast<uint8_t *>(val), 1));

        RESET();
        return ret;
    }


    /**
     * Set the polarity of the INT pin
     *
     * @param val 1 = active high   0 = active low
     * @return
     */
    RetType setInterruptPolarity(lis3mdl_iea_t val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_INT_CFG, reinterpret_cast<uint8_t *>(&val), 1));

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
    I2CAddr_t i2cAddr = {
            .dev_addr = 0x1C << 1,
            .mem_addr = 0x00,
            .mem_addr_size = sizeof(uint8_t),
    };

    RetType initSettings() {
        RESUME();

        // Reset the sensor
        RetType ret = CALL(reset(PROPERTY_ENABLE));

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


};


#endif //LAUNCH_CORE_LIS3MDL_H


