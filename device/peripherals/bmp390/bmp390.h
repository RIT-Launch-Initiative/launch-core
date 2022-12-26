/**
 * Facade for the BMP390 API that utilizes the scheduler
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_BMP390_H
#define LAUNCH_CORE_BMP390_H

#include "device/peripherals/bmp390/bmp3.h"
#include "return.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "device/SPIDevice.h"
#include "device/I2CDevice.h"


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

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
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


        int8_t result = bmp3_init(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType softReset() {
        RESUME();

        int8_t result = bmp3_soft_reset(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setSensorSettings(uint32_t desiredSettings, struct bmp3_settings *settings) {
        RESUME();

        int8_t result = bmp3_set_sensor_settings(desiredSettings, settings, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getPowerMode(uint8_t *opMode) {
        RESUME();

        int8_t result = bmp3_get_op_mode(opMode, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getSensorData(uint8_t sensorComp, struct bmp3_data *data) {
        RESUME();

        int8_t result = bmp3_get_sensor_data(sensorComp, data, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setRegister(uint8_t *regAddress, const uint8_t *regData, uint32_t len) {
        RESUME();

        int8_t result = bmp3_set_regs(regAddress, regData, len, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();

        int8_t result = bmp3_get_regs(regAddress, regData, len, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setFifoSettings(uint16_t desiredSettings, const struct bmp3_fifo_settings *fifo_settings) {
        RESUME();

        int8_t result = bmp3_set_fifo_settings(desiredSettings, fifo_settings, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoSettings(struct bmp3_fifo_settings *fifoSettings) {
        RESUME();

        int8_t result = bmp3_get_fifo_settings(fifoSettings, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoData(struct bmp3_fifo_data *fifo, const struct bmp3_fifo_settings *fifoSettings) {
        RESUME();

        int8_t result = bmp3_get_fifo_data(fifo, fifoSettings, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoLength(uint16_t *fifoLength) {
        RESUME();

        int8_t result = bmp3_get_fifo_length(fifoLength, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType extractFifoData(struct bmp3_data *data, struct bmp3_fifo_data *fifoData) {
        RESUME();

        int8_t result = bmp3_extract_fifo_data(data, fifoData, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setFifoWatermark(const struct bmp3_fifo_data *fifoData,
                             const struct bmp3_fifo_settings *fifoSettings) {
        RESUME();

        int8_t result = bmp3_set_fifo_watermark(fifoData, fifoSettings, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoWatermark(uint16_t *watermarkLength) {
        RESUME();

        int8_t result = bmp3_get_fifo_watermark(watermarkLength, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType flushFifo() {
        RESUME();

        int8_t result = bmp3_fifo_flush(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }


    RetType getStatus(struct bmp3_status *status) {
        RESUME();

        int8_t result = bmp3_get_status(status, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setOperatingMode(bmp3_settings *settings) {
        RESUME();

        int8_t result = bmp3_set_op_mode(settings, &this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

private:
    bmp3_dev device;
    inline static I2CDevice *mI2C;
    inline static SPIDevice *mSPI;

    static BMP3_INTF_RET_TYPE i2cRead(uint8_t regAddr, uint8_t *data, uint32_t len, void *intfPtr) {
        RESUME();

        I2CAddr_t addr = {
                .dev_addr = *static_cast<uint8_t *>(intfPtr),
                .mem_addr = regAddr,
                .mem_addr_size = sizeof(uint8_t),
        };

        static_cast<void>(intfPtr);

        RetType ret = mI2C->read(addr, data, len);
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;

    };

    static BMP3_INTF_RET_TYPE i2cWrite(uint8_t regAddr, const uint8_t *data, uint32_t len, void *intfPtr) {
        RESUME();

        I2CAddr_t addr = {
                .dev_addr = *static_cast<uint8_t *>(intfPtr),
                .mem_addr = regAddr,
                .mem_addr_size = sizeof(uint8_t),
        };

        static_cast<void>(intfPtr);

        RetType ret = mI2C->write(addr, const_cast<uint8_t *>(data), len);
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
