/**
 * LLIS3MDL sensor driver integrated with launch-core scheduler
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

    RetType getRawTemp(int16_t* val) {
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

        RetType ret = CALL(mI2C->read(i2cAddr, data, len);

        RESET();
        return RET_SUCCESS;

    };

    RetType writeReg(uint8_t reg, const uint8_t *data, uint16_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C->write(i2cAddr, data, len));

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

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, (uint8_t *) &ctrlReg1, 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg1.om = val;
        ret = CALL(readReg(LIS3MDL_CTRL_REG4, (uint8_t *) &ctrlReg4, 1));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, (uint8_t *) &ctrlReg1, 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg4.omz = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG4, (uint8_t *) &ctrlReg4, 1));
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

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, static_cast<uint8_t *>(&ctrlReg1), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg1.temp_en = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, static_cast<uint8_t *>(&ctrlReg1), 1));
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

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG2, static_cast<uint8_t *>(&ctrlReg2), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg2.fs = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG2, static_cast<uint8_t *>(&ctrlReg2), 1));

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    /**
     * Sets the operating mode of the magnetometer
     * @param val The value of the MD field in the CTRL_REG3 register
     * @return
     */
    RetType setOperatingMode(lis3mdl_md_t val) {
        RESUME();

        lis3mdl_ctrl_reg3_t ctrlReg3;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG3, static_cast<uint8_t *>(&ctrlReg3), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg3.md = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG3, static_cast<uint8_t *>(&ctrlReg3), 1));

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

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG3, static_cast<uint8_t *>(&ctrlReg3), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg3.lp = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG3, static_cast<uint8_t *>(&ctrlReg3), 1));

        RESET();
        return ret;
    }

    RetType setBlockDataUpdate(uint8_t val) {
        RESUME();

        lis3mdl_ctrl_reg4_t ctrlReg4;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG4, static_cast<uint8_t *>(&ctrlReg4), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg4.bdu = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG4, static_cast<uint8_t *>(&ctrlReg4), 1));

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setHighPartCycle(uint8_t val) {
        RESUME();
        
        RetType ret = CALL(writeReg(LIS3MDL_CTRL_REG5, &val, 1));
        if (ret != RET_SUCCESS) return ret;
        
        if (val == 0) {
            lis3mdl_ctrl_reg5_t ctrlReg5;
            ret = CALL(readReg(LIS3MDL_CTRL_REG5, static_cast<uint8_t *>(&ctrlReg5), 1));
            if (ret != RET_SUCCESS) return ret;
        }

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setSelfTest(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_self_test_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setReset(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_reset_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setBoot(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_boot_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setDataFormat(lis3mdl_ble_t val) {
        RESUME();

        int32_t result = lis3mdl_data_format_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setConfig(lis3mdl_int_cfg_t *val) { // Possible error made by the library?
        RESUME();

        int32_t result = lis3mdl_int_config_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setGeneration(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_int_generation_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getGeneration(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_generation_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setPolarity(lis3mdl_iea_t val) {
        RESUME();

        int32_t result = lis3mdl_int_polarity_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getPolarity(lis3mdl_iea_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_polarity_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setZAxis(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_int_on_z_ax_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getZAxis(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_on_z_ax_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setYAxis(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_int_on_y_ax_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getYAxis(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_on_y_ax_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setXAxis(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_int_on_x_ax_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getXAxis(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_on_x_ax_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setThreshold(uint16_t val) {
        RESUME();

        int32_t result = lis3mdl_int_threshold_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getThreshold(uint16_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_threshold_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setNotificationMode(lis3mdl_lir_t val) {
        RESUME();

        int32_t result = lis3mdl_int_notification_mode_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getNotificationMode(lis3mdl_lir_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_notification_mode_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setSPIMode(lis3mdl_sim_t val) {
        RESUME();

        int32_t result = lis3mdl_spi_mode_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getSPIMode(lis3mdl_sim_t *val) {
        RESUME();

        int32_t result = lis3mdl_spi_mode_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }


    RetType getMagDataReady(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_mag_data_ready_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getMagDataOvr(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_mag_data_ovr_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getRawMagnetic(int16_t *val) {
        RESUME();

        int32_t result = lis3mdl_magnetic_raw_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getRawTemperature(int16_t *val) {
        RESUME();

        int32_t result = lis3mdl_temperature_raw_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getDeviceID(uint8_t *buff) {
        RESUME();

        int32_t result = lis3mdl_device_id_get(&device, buff);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getSource(lis3mdl_int_src_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_source_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getInterruptEventFlag(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_interrupt_event_flag_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getMagOverRangeFlag(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_mag_over_range_flag_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getNegZFlag(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_neg_z_flag_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getNegYFlag(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_neg_y_flag_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getNegXFlag(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_neg_x_flag_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getPosZFlag(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_pos_z_flag_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getPosYFlag(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_pos_y_flag_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getPosXFlag(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_pos_x_flag_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }


    RetType getStatus(lis3mdl_status_reg_t *val) {
        RESUME();

        int32_t result = lis3mdl_status_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }


private:
    I2CDevice &mI2C;
    I2CAddr_t i2cAddr = {
            .dev_addr = LIS3MDL_I2C_ADD_L,
            .mem_addr = 0x00,
            .mem_addr_size = sizeof(uint8_t),
    };
};


#endif //LAUNCH_CORE_LIS3MDL_H


