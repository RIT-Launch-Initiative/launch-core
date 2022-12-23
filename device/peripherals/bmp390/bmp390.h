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


class BMP390 {
public:
    BMP390(uint8_t chipID, void *pInterface, bmp3_intf commInterface, struct bmp3_calib_data calibrationData,
           bmp3_read_fptr_t readFptr, bmp3_write_fptr_t writePtr, bmp3_delay_us_fptr_t delayPtr) :
            device({
                           .chip_id = chipID,
                           .intf_ptr = pInterface,
                           .intf = commInterface,
                           .intf_rslt = BMP3_INTF_RET_SUCCESS,
                           .dummy_byte = 0, // TODO: Figure dummy bytes out
                           .read = readFptr,
                           .write = writePtr,
                           .delay_us = delayPtr,
                           .calib_data = calibrationData,
                   }) {
    }

    RetType init() {
        RESUME();

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

    RetType setSensorSettings(struct bmp3_settings *settings) {
        RESUME();

        int8_t result = bmp3_get_sensor_settings(settings, &this->device);

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

    RetType setSensorSettings() {
        RESUME();

        int8_t result = bmp3_set_sensor_settings(&this->device);

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

    RetType getFifoData(struct bmp3_fifo_data *fifo,
                        const struct bmp3_fifo_settings *fifoSettings) {
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

private:
    bmp3_dev device;

    RetType bmpResultConvert(int8_t result) {
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }
};

#endif //LAUNCH_CORE_BMP390_H
