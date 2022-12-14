/**
 * Facade for the BMP390 API that utilizes the scheduler
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_BMP390_H
#define LAUNCH_CORE_BMP390_H

#include "device/peripherals/BMP390/bmp3.h"
#include "return.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "device/SPIDevice.h"
#include "device/I2CDevice.h"
#include "sched/macros/call.h"


class BMP390 {
public:
    BMP390(void *pInterface, bmp3_intf commInterface, struct bmp3_calib_data calibrationData,
           bmp3_delay_us_fptr_t delayFptr, SPIDevice *spiDev = nullptr, I2CDevice *i2cDev = nullptr) :
            device({.intf_ptr = pInterface, .intf = commInterface, .delay_us = delayFptr,}) {
        mSPI = spiDev;
        mI2C = i2cDev;
    }

    BMP390(void *pInterface, struct bmp3_calib_data calibrationData, bmp3_delay_us_fptr_t delayFptr,
           I2CDevice *i2cDev = nullptr) :
            device({.intf_ptr = pInterface, .intf = BMP3_I2C_INTF, .delay_us = delayFptr,}) {
        mI2C = i2cDev;
    }

    BMP390(void *pInterface, struct bmp3_calib_data calibrationData, bmp3_delay_us_fptr_t delayFptr, SPIDevice *spiDev)
            :
            device({.intf_ptr = pInterface, .intf = BMP3_SPI_INTF, .delay_us = delayFptr,}) {
        mSPI = spiDev;
    }

    /*************************************************************************************
     * Main Functionality
     *************************************************************************************/

    // Provide own functions for now
    RetType init(BMP3_INTF_RET_TYPE (*readFptr)(uint8_t, uint8_t *, uint32_t, void *),
                 BMP3_INTF_RET_TYPE (writeFptr)(uint8_t, const uint8_t *, uint32_t, void *)) {
        RESUME();

        switch (device.intf) {
            case BMP3_SPI_INTF:
                if (mSPI == nullptr) return RET_ERROR;

                device.read = spiRead;
                device.write = spiWrite;

                break;
            case BMP3_I2C_INTF:
                if (mI2C == nullptr) return RET_ERROR;

                device.read = readFptr;
                device.write = writeFptr;

                break;
            default:
                return RET_ERROR;
        }


        int8_t result = bmp3_init(&this->device);
        if (result != BMP3_OK) return RET_ERROR;

        result = initSettings();

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    // TODO: Provided fn ptrs having issues with scheduler
    RetType init() {
        RESUME();

        switch (device.intf) {
            case BMP3_SPI_INTF:
                if (mSPI == nullptr) return RET_ERROR;

                device.read = spiRead;
                device.write = spiWrite;

                break;
            case BMP3_I2C_INTF:
                if (mI2C == nullptr) return RET_ERROR;

                device.read = i2cRead;
                device.write = i2cWrite;

                break;
            default:
                return RET_ERROR;
        }


//        int8_t result = bmp3_init(&this->device);
//        if (result != BMP3_OK) return RET_ERROR;
        uint8_t chipID;
        I2CAddr_t addr = {
                .dev_addr = static_cast<uint16_t>((*reinterpret_cast<uint8_t *>(&device)) << 1),
                .mem_addr = 0x00,
                .mem_addr_size = 0x00000001U,
        };
        RetType ret = mI2C->read(addr, &chipID, 1);


//        result = initSettings();


        RESET();
        return RET_SUCCESS;
    }

    RetType getSensorData(uint8_t sensorComp, struct bmp3_data *data) {
        RESUME();

        int8_t result = bmp3_get_sensor_data(sensorComp, data, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    RetType softReset() {
        RESUME();

        int8_t result = bmp3_soft_reset(&this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }


    RetType setRegister(uint8_t *regAddress, const uint8_t *regData, uint32_t len) {
        RESUME();

        int8_t result = bmp3_set_regs(regAddress, regData, len, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }


    RetType getRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();

        int8_t result = bmp3_get_regs(regAddress, regData, len, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    /*************************************************************************************
     * FIFO
     *************************************************************************************/

    RetType getFifoData(struct bmp3_fifo_data *fifo, const struct bmp3_fifo_settings *fifoSettings) {
        RESUME();

        int8_t result = bmp3_get_fifo_data(fifo, fifoSettings, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoLength(uint16_t *fifoLength) {
        RESUME();

        int8_t result = bmp3_get_fifo_length(fifoLength, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoWatermark(uint16_t *watermarkLength) {
        RESUME();

        int8_t result = bmp3_get_fifo_watermark(watermarkLength, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    RetType getStatus(struct bmp3_status *status) {
        RESUME();

        int8_t result = bmp3_get_status(status, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    RetType extractFifoData(struct bmp3_data *data, struct bmp3_fifo_data *fifoData) {
        RESUME();

        int8_t result = bmp3_extract_fifo_data(data, fifoData, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    RetType flushFifo() {
        RESUME();

        int8_t result = bmp3_fifo_flush(&this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    /*************************************************************************************
     * Settings
     *************************************************************************************/

    RetType setSensorSettings(uint32_t desiredSettings, struct bmp3_settings *settings) {
        RESUME();

        int8_t result = bmp3_set_sensor_settings(desiredSettings, settings, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    RetType setOperatingMode(bmp3_settings *settings) {
        RESUME();

        int8_t result = bmp3_set_op_mode(settings, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    RetType getPowerMode(uint8_t *opMode) {
        RESUME();

        int8_t result = bmp3_get_op_mode(opMode, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }


    RetType setFifoSettings(uint16_t desiredSettings, const struct bmp3_fifo_settings *fifo_settings) {
        RESUME();

        int8_t result = bmp3_set_fifo_settings(desiredSettings, fifo_settings, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }


    RetType getFifoSettings(struct bmp3_fifo_settings *fifoSettings) {
        RESUME();

        int8_t result = bmp3_get_fifo_settings(fifoSettings, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }


    RetType setFifoWatermarkSettings(const struct bmp3_fifo_data *fifoData,
                                     const struct bmp3_fifo_settings *fifoSettings) {
        RESUME();

        int8_t result = bmp3_set_fifo_watermark(fifoData, fifoSettings, &this->device);

        RESET();
        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

private:
    bmp3_dev device;
    inline static I2CDevice *mI2C;
    inline static SPIDevice *mSPI;

    RetType initSettings() {
        bmp3_settings settings = {
                .op_mode = BMP3_MODE_NORMAL,
                .press_en = BMP3_ENABLE,
                .temp_en = BMP3_ENABLE,
                .odr_filter = {
                        .press_os = BMP3_OVERSAMPLING_2X,
                        .temp_os = BMP3_OVERSAMPLING_2X,
                        .iir_filter = BMP3_IIR_FILTER_DISABLE,
                        .odr = BMP3_ODR_100_HZ
                },
                .int_settings = {
                        .output_mode = BMP3_INT_PIN_PUSH_PULL,
                        .level = BMP3_INT_PIN_ACTIVE_LOW,
                        .latch = BMP3_INT_PIN_NON_LATCH,
                        .drdy_en = BMP3_ENABLE
                },
                .adv_settings = {
                        .i2c_wdt_en = BMP3_DISABLE,
                        .i2c_wdt_sel = BMP3_DISABLE
                }
        };

        uint16_t settingsSel = BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN | BMP3_SEL_PRESS_OS |
                               BMP3_SEL_TEMP_OS | BMP3_SEL_ODR | BMP3_SEL_DRDY_EN;
        int result = setSensorSettings(settingsSel, &settings);
        if (result != BMP3_OK) return RET_ERROR;

        result = setOperatingMode(&settings);

        return result == BMP3_OK ? RET_SUCCESS : RET_ERROR;
    }

    static BMP3_INTF_RET_TYPE i2cRead(uint8_t regAddr, uint8_t *data, uint32_t len, void *intfPtr) {
        RESUME();

        I2CAddr_t addr = {
                .dev_addr = static_cast<uint16_t>((*static_cast<uint8_t *>(intfPtr)) << 1),
                .mem_addr = regAddr,
                .mem_addr_size = 0x00000001U,
        };

        static_cast<void>(intfPtr);

        RetType ret = CALL(mI2C->read(addr, data, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;

    };

    static BMP3_INTF_RET_TYPE i2cWrite(uint8_t regAddr, const uint8_t *data, uint32_t len, void *intfPtr) {
        RESUME();

        I2CAddr_t addr = {
                .dev_addr = static_cast<uint16_t>((*static_cast<uint8_t *>(intfPtr)) << 1),
                .mem_addr = regAddr,
                .mem_addr_size = 0x00000001U,
        };

        static_cast<void>(intfPtr);

        RetType ret = CALL(mI2C->write(addr, const_cast<uint8_t *>(data), len));
        if (ret != RET_SUCCESS) return ret; // TODO: Should map to BMP3_INTF_RET_TYPE if possible

        RESET();
        return RET_SUCCESS;
    };

// TODO: Not using SPI, but should figure out how to use regAddr here
    static BMP3_INTF_RET_TYPE spiRead(uint8_t regAddr, uint8_t *data, uint32_t len, void *intfPtr) {
        RESUME();
        RetType ret = mSPI->read(data, len);
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    };

    static BMP3_INTF_RET_TYPE spiWrite(uint8_t regAddr, const uint8_t *data, uint32_t len, void *intfPtr) {
        RESUME();

        RetType ret = mSPI->read(const_cast<uint8_t *>(data), len);
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;

    };
};


#endif //LAUNCH_CORE_BMP390_H
