/**
 * Facade for the L3GD20H STM driver using launch-core
 *
 * NOTE: Will be untested and most likely unused. Going out of production.
 *
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_L3GD20H_H
#define LAUNCH_CORE_L3GD20H_H

#include "device/peripherals/L3GD20H/l3gd20h_reg.h"
#include "device/I2CDevice.h"
#include "device/SPIDevice.h"
#include "sched/macros/resume.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"

enum COMM_MODE { // Please just use I2C for now
    SPI,
    I2C
};

class L3GD20H {
public:
    L3GD20H(void *pInterface, stmdev_mdelay_ptr delayFptr,
            SPIDevice *spiDev = nullptr, I2CDevice *i2cDev = nullptr) :
            device({
                           .mdelay = delayFptr
                   }) {
        mSPI = spiDev;
        mI2C = i2cDev;
    };

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

    RetType readReg(uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        int32_t result = l3gd20h_read_reg(&device, reg, data, len);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType writeReg(uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        int32_t result = l3gd20h_write_reg(&device, reg, data, len);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType fs245ToMDPS(int16_t lsb, float_t *result) {
        RESUME();

        *result = l3gd20h_from_fs245_to_mdps(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType fs500ToMDPS(int16_t lsb, float_t *result) {
        RESUME();

        *result = l3gd20h_from_fs500_to_mdps(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType fs2000ToMDPS(int16_t lsb, float_t *result) {
        RESUME();

        *result = l3gd20h_from_fs2000_to_mdps(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType lsbToCelsius(int16_t lsb, float_t *result) {
        RESUME();

        *result = l3gd20h_from_lsb_to_celsius(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroAxis(l3gd20h_gy_axis_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_axis_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroAxis(l3gd20h_gy_axis_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_axis_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setDataRate(l3gd20h_gy_data_rate_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_data_rate_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getDataRate(l3gd20h_gy_data_rate_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_data_rate_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setFullScale(l3gd20h_gy_fs_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_full_scale_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFullScale(l3gd20h_gy_fs_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_full_scale_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setBlockDataUpdate(uint8_t val) {
        RESUME();

        int32_t result = l3gd20h_block_data_update_set(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getBlockDataUpdate(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_block_data_update_get(&device, val);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setDataFormat(l3gd20h_ble_t val) {
        RESUME();

        int32_t result = l3gd20h_dev_data_format_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getDataFormat(l3gd20h_ble_t *val) {
        RESUME();

        int32_t result = l3gd20h_dev_data_format_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType set(uint8_t val) {
        RESUME();

        int32_t result = (&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType setBoot(uint8_t val) {
        RESUME();

        int32_t result = l3gd20h_dev_boot_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getBoot(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_dev_boot_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setReset(uint8_t val) {
        RESUME();

        int32_t result = l3gd20h_dev_reset_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getReset(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_dev_reset_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setLPFilterBandwith(l3gd20h_lpbw_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_lp_bandwidth_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getLPFilterBandwith(l3gd20h_lpbw_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_lp_bandwidth_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setHPFilterBandwith(l3gd20h_gy_hp_bw_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_hp_bandwidth_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getHPFilterBandwith(l3gd20h_gy_hp_bw_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_hp_bandwidth_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setFilterOutPath(l3gd20h_gy_out_path_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_out_path_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFilterOutPath(l3gd20h_gy_out_path_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_out_path_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setFilterIntPath(l3gd20h_gy_int_path_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_int_path_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFilterIntPath(l3gd20h_gy_int_path_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_int_path_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setFilterReference(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_reference_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFilterReference(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_filter_reference_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setSPIMode(l3gd20h_sim_t val) {
        RESUME();

        int32_t result = l3gd20h_spi_mode_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getSPIMode(l3gd20h_sim_t *val) {
        RESUME();

        int32_t result = l3gd20h_spi_mode_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setI2CInterface(l3gd20h_i2c_dis_t val) {
        RESUME();

        int32_t result = l3gd20h_i2c_interface_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getI2CInterface(l3gd20h_i2c_dis_t *val) {
        RESUME();

        int32_t result = l3gd20h_i2c_interface_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setPinInt1Route(l3gd20h_pin_int1_rt_t val) {
        RESUME();

        int32_t result = l3gd20h_pin_int1_route_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getPinInt1Route(l3gd20h_pin_int1_rt_t *val) {
        RESUME();

        int32_t result = l3gd20h_pin_int1_route_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setPinInt2Route(l3gd20h_pin_int2_rt_t val) {
        RESUME();

        int32_t result = l3gd20h_pin_int2_route_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getPinInt2Route(l3gd20h_pin_int2_rt_t *val) {
        RESUME();

        int32_t result = l3gd20h_pin_int2_route_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setPinMode(l3gd20h_pp_od_t val) {
        RESUME();

        int32_t result = l3gd20h_pin_mode_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getPinMode(l3gd20h_pp_od_t *val) {
        RESUME();

        int32_t result = l3gd20h_pin_mode_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setPinPolarity(l3gd20h_pin_pol_t val) {
        RESUME();

        int32_t result = l3gd20h_pin_polarity_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getPinPolarity(l3gd20h_pin_pol_t *val) {
        RESUME();

        int32_t result = l3gd20h_pin_polarity_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setPinNotification(l3gd20h_lir_t val) {
        RESUME();

        int32_t result = l3gd20h_pin_notification_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getPinNotification(l3gd20h_lir_t *val) {
        RESUME();

        int32_t result = l3gd20h_pin_notification_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setPinLogic(l3gd20h_pin_logic_t val) {
        RESUME();

        int32_t result = l3gd20h_pin_logic_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getPinLogic(l3gd20h_pin_logic_t *val) {
        RESUME();

        int32_t result = l3gd20h_pin_logic_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroXThreshold(uint16_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_x_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroXThreshold(uint16_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_x_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroYThreshold(uint16_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_y_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroYThreshold(uint16_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_y_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroZThreshold(uint16_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_z_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroZThreshold(uint16_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_z_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroThresholdAxis(l3gd20h_gy_trshld_en_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_axis_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroThresholdAxis(l3gd20h_gy_trshld_en_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_axis_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroModeThreshold(l3gd20h_dcrm_g_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_mode_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroModeThreshold(l3gd20h_dcrm_g_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_mode_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setGyroMinSampleThreshold(uint8_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_min_sample_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroMinSampleThreshold(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_min_sample_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setFifoStopWtm(uint8_t val) {
        RESUME();

        int32_t result = l3gd20h_fifo_stop_on_wtm_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFifoStopWtm(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_fifo_stop_on_wtm_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setFifoMode(l3gd20h_fifo_m_t val) {
        RESUME();

        int32_t result = l3gd20h_fifo_mode_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFifoMode(l3gd20h_fifo_m_t *val) {
        RESUME();

        int32_t result = l3gd20h_fifo_mode_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setFifoWatermark(uint8_t val) {
        RESUME();

        int32_t result = l3gd20h_fifo_watermark_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFifoWatermark(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_fifo_watermark_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setDenMode(l3gd20h_den_md_t val) {
        RESUME();

        int32_t result = l3gd20h_den_mode_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getDenMode(l3gd20h_den_md_t *val) {
        RESUME();

        int32_t result = l3gd20h_den_mode_get(&device, val);
        if (result != 0) return RET_ERROR;


        RESET();
        return RET_SUCCESS;
    }

    RetType setSelfTest(l3gd20h_st_t val) {
        RESUME();

        int32_t result = l3gd20h_gy_self_test_set(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getSelfTest(l3gd20h_st_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_self_test_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroFlagDataReady(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_flag_data_ready_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getRawTemp(uint8_t *buff) {
        RESUME();

        int32_t result = l3gd20h_temperature_raw_get(&device, buff);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getRawAngularRate(int16_t *val) {
        RESUME();

        int32_t result = l3gd20h_angular_rate_raw_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getDeviceID(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_dev_id_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getDeviceStatus(l3gd20h_status_reg_t *val) {
        RESUME();

        int32_t result = l3gd20h_dev_status_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getGyroscopeThreshold(l3gd20h_gy_trshld_src_t *val) {
        RESUME();

        int32_t result = l3gd20h_gy_trshld_src_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFifoSrc(l3gd20h_fifo_srs_t *val) {
        RESUME();

        int32_t result = l3gd20h_fifo_src_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFifoDataLevel(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_fifo_data_level_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFifoFullFlag(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_fifo_full_flag_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType getFifoWtmFlag(uint8_t *val) {
        RESUME();

        int32_t result = l3gd20h_fifo_wtm_flag_get(&device, val);
        if (result != 0) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

private:
    static I2CDevice *mI2C;
    static SPIDevice *mSPI;
    stmdev_ctx_t device;

    static int32_t i2cRead(void *handle, uint8_t reg, uint8_t *data, uint16_t len) {
        RESUME();

        I2CAddr_t addr = {
                .dev_addr = L3GD20H_I2C_ADD_L,
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
                .dev_addr = L3GD20H_I2C_ADD_L,
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

#endif //LAUNCH_CORE_L3GD20H_H
