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
    BMP390() {}

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

    RetType getFifoSettings() {
        RESUME();

        int8_t result = bmp3_get_fifo_settings(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoData() {
        RESUME();

        int8_t result = bmp3_get_fifo_data(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoLength() {
        RESUME();

        int8_t result = bmp3_get_fifo_length(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType extractFifoData() {
        RESUME();

        int8_t result = bmp3_extract_fifo_data(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType setFifoWatermark() {
        RESUME();

        int8_t result = bmp3_set_fifo_watermark(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType getFifoWatermark() {
        RESUME();

        int8_t result = bmp3_get_fifo_watermark(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType flushFifo() {
        RESUME();

        int8_t result = bmp3_fifo_flush(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }


    RetType getStatus() {
        RESUME();

        int8_t result = bmp3_get_status(&this->device);

        RESET();
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

private:
    bmp3_dev device;
    int8_t chip_id;


    void *intf_ptr;

    enum bmp3_intf commInterface;

    BMP3_INTF_RET_TYPE interfaceResult;

    RetType bmpResultConvert(int8_t result) {
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }
};

#endif //LAUNCH_CORE_BMP390_H
