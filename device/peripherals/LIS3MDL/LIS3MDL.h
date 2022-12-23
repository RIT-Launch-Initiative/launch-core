/**
 * Facade for using LIS3MDL sensor driver with launch-core scheduler
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LIS3MDL_H
#define LAUNCH_CORE_LIS3MDL_H

#include "device/I2CDevice.h"
#include "device/SPIDevice.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "lis3mdl_reg.h"
#include "sched/macros/call.h"

enum COMM_MODE { // Please just use I2C for now
    SPI,
    I2C
};

class LIS3MDL {
public:
    LIS3MDL(uint8_t chipID, void *pInterface, stmdev_mdelay_ptr delayFptr,
            SPIDevice *spiDev = nullptr, I2CDevice *i2cDev = nullptr) :
            device({
                           .mdelay = delayFptr
                   }) {
        mSPI = spiDev;
        mI2C = i2cDev;
    }

    RetType init(COMM_MODE mode) {
        RESUME();

        switch (mode) {
            case SPI:
                device.write_reg = spiWrite;
                device.read_reg = spiRead;
            case I2C:
                device.write_reg = i2cWrite;
                device.read_reg = i2cRead;
                break;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType readRegister(uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        int32_t result = lis3mdl_read_reg(&device, reg, data, len);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType writeRegister(uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        int32_t result = lis3mdl_write_reg(&device, reg, data, len);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }


    RetType fs4ToGauss(int16_t lsb, float_t *gauss) {
        RESUME();

        *gauss = lis3mdl_from_fs4_to_gauss(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType fs8ToGauss(int16_t lsb, float_t *gauss) {
        RESUME();

        *gauss = lis3mdl_from_fs8_to_gauss(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType fs12ToGauss(int16_t lsb, float_t *gauss) {
        RESUME();

        *gauss = lis3mdl_from_fs12_to_gauss(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType fs16ToGauss(int16_t lsb, float_t *gauss) {
        RESUME();

        *gauss = lis3mdl_from_fs16_to_gauss(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType lsbToGauss(int16_t lsb, float_t *gauss) {
        RESUME();

        *gauss = lis3mdl_from_lsb_to_celsius(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType setDataRate(lis3mdl_om_t val) {
        RESUME();

        int32_t result = lis3mdl_data_rate_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getDataRate(lis3mdl_om_t *val) {
        RESUME();

        int32_t result = lis3mdl_data_rate_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setTempMeas(uint8_t val) {
        RESUME();

        int32_t result = lis3mdl_temperature_meas_set(&device, val);

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
    static I2CDevice *mI2C;
    static SPIDevice *mSPI;
    stmdev_ctx_t device;

    static int32_t i2cRead(void *handle, uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        I2CAddr_t addr = {
                .dev_addr = LIS3MDL_I2C_ADD_L,
                .mem_addr = reg,
                .mem_addr_size = sizeof(uint8_t),
        };

        RetType ret = CALL(mI2C->read(addr, const_cast<uint8_t *>(data), len));

        RESET();
        return ret == RET_SUCCESS ? 0 : -1;

    };

    static int32_t i2cWrite(void *handle, uint8_t reg, const uint8_t *data, uint16_t len) {
        RESUME();

        I2CAddr_t addr = {
                .dev_addr = LIS3MDL_I2C_ADD_L,
                .mem_addr = reg,
                .mem_addr_size = sizeof(uint8_t),
        };

        RetType ret = CALL(mI2C->write(addr, const_cast<uint8_t *>(data), len));

        RESET();
        return ret == RET_SUCCESS ? 0 : -1;
    };

    // TODO: Not using SPI, but should figure out how to use regAddr here
    // Would need to also use cs pin and do both tx and rx
    static int32_t spiRead(void *handle, uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        RetType ret = CALL(mSPI->read(const_cast<uint8_t *>(data), len));

        RESET();
        return ret == RET_SUCCESS ? 0 : -1;
    };

    static int32_t spiWrite(void *handle, uint8_t reg, const uint8_t *data, uint16_t len) {
        RESUME();

        RetType ret = CALL(mSPI->read(const_cast<uint8_t *>(data), len));

        RESET();
        return ret == RET_SUCCESS ? 0 : -1;

    };
};


#endif //LAUNCH_CORE_LIS3MDL_H
