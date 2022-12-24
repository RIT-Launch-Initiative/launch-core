/**
 * Facade for the L3GD20H STM driver using launch-core
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
