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
            }){
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


    RetType fs4ToGauss(int16_t lsb, float_t* gauss) {
        RESUME();

        *gauss = lis3mdl_from_fs4_to_gauss(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType fs8ToGauss(int16_t lsb, float_t* gauss) {
        RESUME();

        *gauss = lis3mdl_from_fs8_to_gauss(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType fs12ToGauss(int16_t lsb, float_t* gauss) {
        RESUME();

        *gauss = lis3mdl_from_fs12_to_gauss(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType fs16ToGauss(int16_t lsb, float_t* gauss) {
        RESUME();

        *gauss = lis3mdl_from_fs16_to_gauss(lsb);

        RESET();
        return RET_SUCCESS;
    }

    RetType lsbToGauss(int16_t lsb, float_t* gauss) {
        RESUME();

        *gauss = lis3mdl_from_lsb_to_celsius(lsb);

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
