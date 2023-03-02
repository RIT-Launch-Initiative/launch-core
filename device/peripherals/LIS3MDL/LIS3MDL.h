/**
 * LIS3MDL sensor driver integrated with launch-core scheduler
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LIS3MDL_H
#define LAUNCH_CORE_LIS3MDL_H

#include "device/I2CDevice.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "lis3mdl_reg.h"
#include "sched/macros/call.h"

class LIS3MDL {
public:
    LIS3MDL(I2CDevice &i2cDevice) : mI2C(&i2cDevice) {}

    RetType init() {
        RESUME();

        uint8_t whoAmI = 0;
        RetType ret = CALL(readReg(LIS3MDL_WHO_AM_I, &whoAmI, 1));
        if (ret != RET_SUCCESS) return ret;

        if (whoAmI != LIS3MDL_ID) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getRawMagnetic(int16_t *val) {
        RESUME();

        uint8_t data[6];
        RetType ret = CALL(readReg(LIS3MDL_OUT_X_L, data, 6));
        if (ret != RET_SUCCESS) return ret;

        val[0] = (int16_t) data[1];
        val[0] = (val[0] * 256) + (int16_t) data[0];
        val[1] = (int16_t) data[3];
        val[1] = (val[1] * 256) + (int16_t) data[2];
        val[2] = (int16_t) data[5];
        val[2] = (val[2] * 256) + (int16_t) data[4];

        RESET();
        return RET_SUCCESS;
    }

    RetType getRawTemp(int16_t *val) {
        RESUME();

        uint8_t data[2];
        RetType ret = CALL(readReg(LIS3MDL_TEMP_OUT_L, data, 2));
        if (ret != RET_SUCCESS) return ret;

        val[0] = (int16_t) data[1];
        val[0] = (val[0] * 256) + (int16_t) data[0];

        RESET();
        return RET_SUCCESS;
    }

    RetType readReg(uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;

        RetType ret = CALL(mI2C.read(i2cAddr, data, len));

        RESET();
        return RET_SUCCESS;

    };

    RetType writeReg(uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C.write(i2cAddr, data, len));

        RESET();
        return RET_SUCCESS;
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

        lis3mdl_ctrl_reg1_t ctrlReg1;
        lis3mdl_ctrl_reg4_t ctrlReg4;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, (uint8_t * ) & ctrlReg1, 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg1.om = val;
        ret = CALL(readReg(LIS3MDL_CTRL_REG4, (uint8_t * ) & ctrlReg4, 1));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, (uint8_t * ) & ctrlReg1, 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg4.omz = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG4, (uint8_t * ) & ctrlReg4, 1));
        if (ret != RET_SUCCESS) return ret;

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

        lis3mdl_ctrl_reg1_t ctrlReg1;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, reinterpret_cast<uint8_t *>(&ctrlReg1), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg1.temp_en = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, reinterpret_cast<uint8_t *>(&ctrlReg1), 1));
        if (ret != RET_SUCCESS) return ret;

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

        lis3mdl_ctrl_reg2_t ctrlReg2;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&ctrlReg2), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg2.fs = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&ctrlReg2), 1));

        RESET();
        return RET_SUCCESS;
    }

    /**
     * Sets the operating mode of the magnetometer
     * @param val The value of the MD field in the CTRL_REG3 register
     * @return
     */
    RetType setOperatingMode(lis3mdl_md_t val) {
        RESUME();

        lis3mdl_ctrl_reg3_t ctrlReg3;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&ctrlReg3), 1));
        if (ret != RET_SUCCESS) return ret;

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

        lis3mdl_ctrl_reg3_t ctrlReg3;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&ctrlReg3), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg3.lp = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG3, reinterpret_cast<uint8_t *>(&ctrlReg3), 1));

        RESET();
        return ret;
    }

    RetType setBlockDataUpdate(uint8_t val) {
        RESUME();

        lis3mdl_ctrl_reg5_t ctrlReg5;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&ctrlReg5), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg5.bdu = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&ctrlReg5), 1));

        RESET();
        return RET_SUCCESS;

    }

    RetType setHighPartCycle(uint8_t val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_CTRL_REG5, &val, 1));
        if (ret != RET_SUCCESS) return ret;

        if (val == 0) {
            lis3mdl_ctrl_reg5_t ctrlReg5;
            ret = CALL(readReg(LIS3MDL_CTRL_REG5, reinterpret_cast<uint8_t *>(&ctrlReg5), 1));
            if (ret != RET_SUCCESS) return ret;
        }

        RESET();
        return RET_SUCCESS;
    }


    /**
     * Software reset. Restore default values in user registers
     *
     * @param val
     * @return
     */
    RetType reset(uint8_t val) {
        RESUME();
        lis3mdl_ctrl_reg2_t ctrlReg2;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&ctrlReg2), 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType reboot(uint8_t val) {
        RESUME();
        lis3mdl_ctrl_reg2_t ctrlReg2;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, reinterpret_cast<uint8_t *>(&ctrlReg2), 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    /**
     * Set the endianness of the data output
     *
     * @param val 1 = little endian   0 = big endian
     * @return
     */
    RetType setDataEndianness(uint8_t val) {
        RESUME();
        lis3mdl_ctrl_reg4_t ctrlReg4;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&ctrlReg4), 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    /**
     * Get the endianness of the data output
     *
     * @param val 1 = little endian   0 = big endian
     * @return
     */
    RetType getDataEndianness(uint8_t *val) {
        RESUME();
        lis3mdl_ctrl_reg4_t ctrlReg4;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, reinterpret_cast<uint8_t *>(&ctrlReg4), 1));
        if (ret != RET_SUCCESS) return ret;

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
        return RET_SUCCESS;
    }

    RetType setConfig(lis3mdl_int_cfg_t *val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_INT_CFG, reinterpret_cast<uint8_t *>(val), 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    /**
     * Enable INT pin
     * @param val 1 = enable   0 = disable
     * @return
     */
    RetType enableInterrupt(uint8_t val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_INT_CFG, &val, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType setInterruptNotifications(lis3mdl_lir_t *val) {
        RESUME();

        RetType ret = CALL(writeReg(LIS3MDL_INT_SRC, reinterpret_cast<uint8_t *>(val), 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
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
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }


    RetType getStatus(lis3mdl_status_reg_t *val) {
        RESUME();

        RetType ret = CALL(readReg(LIS3MDL_STATUS_REG, reinterpret_cast<uint8_t *>(val), 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }


private:
    I2CDevice &mI2C;
    I2CAddr_t i2cAddr = {
            .dev_addr = 0x1C << 1,
            .mem_addr = 0x00,
            .mem_addr_size = sizeof(uint8_t),
    };
};


#endif //LAUNCH_CORE_LIS3MDL_H


