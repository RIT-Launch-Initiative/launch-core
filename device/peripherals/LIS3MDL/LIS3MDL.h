/**
 * Facade for using LIS3MDL sensor driver with launch-core scheduler
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

    RetType readReg(void *handle, uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;

        RetType ret = CALL(mI2C->read(i2cAddr, data, len);

        RESET();
        return RET_SUCCESS;

    };

    RetType writeReg(void *handle, uint8_t reg, const uint8_t *data, uint16_t len) {
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

    RetType setTempMeas(uint8_t val) {
        RESUME();

        lis3mdl_ctrl_reg1_t ctrlReg1;

        RetType ret = CALL(readReg(LIS3MDL_CTRL_REG1, static_cast<uint8_t *>(&ctrlReg1), 1));
        if (ret != RET_SUCCESS) return ret;

        ctrlReg1.temp_en = val;
        ret = CALL(writeReg(LIS3MDL_CTRL_REG1, static_cast<uint8_t *>(&ctrlReg1), 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getTempMeas(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_temperature_meas_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }


    RetType setFullScale(lis3mdl_fs_t val) {
        RESUME();

        int32_t result = lis3mdl_full_scale_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFullScale(lis3mdl_fs_t *val) {
        RESUME();

        int32_t result = lis3mdl_full_scale_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setOperatingMode(lis3mdl_md_t val) {
        RESUME();

        int32_t result = lis3mdl_operating_mode_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getOperatingMode(lis3mdl_md_t *val) {
        RESUME();

        int32_t result = lis3mdl_operating_mode_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setLowPower(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_fast_low_power_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getLowPower(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_fast_low_power_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setBlockDataUpdate(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_block_data_update_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getBlockDataUpdate(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_block_data_update_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setHighPartCycle(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_high_part_cycle_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getHighPartCycle(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_high_part_cycle_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setSelfTest(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_self_test_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getSelfTest(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_self_test_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setReset(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_reset_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getReset(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_reset_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setBoot(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_boot_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getBoot(uint8_t *val) {
        RESUME();

        int32_t result = lis3mdl_boot_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setDataFormat(lis3mdl_ble_t val) {
        RESUME();

        int32_t result = lis3mdl_data_format_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getDataFormat(lis3mdl_ble_t *val) {
        RESUME();

        int32_t result = lis3mdl_data_format_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setConfig(lis3mdl_int_cfg_t *val) { // Possible error made by the library?
        RESUME();

        int32_t result = lis3mdl_int_config_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getConfig(lis3mdl_int_cfg_t *val) {
        RESUME();

        int32_t result = lis3mdl_int_config_get(&device, val);

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


