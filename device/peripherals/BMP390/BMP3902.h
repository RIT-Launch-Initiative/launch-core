/**
 * Facade for the BMP390 API that utilizes the scheduler
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_BMP390_H
#define LAUNCH_CORE_BMP390_H

#define INTERRUPT_WAIT_TIME 25000

#include "device/peripherals/BMP390/bmp3.h"
#include "return.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "device/SPIDevice.h"
#include "device/I2CDevice.h"
#include "sched/macros/call.h"
#include "sched/macros/sleep.h"


class BMP390 {
public:
    BMP390(I2CDevice *i2cDev) : mI2C(i2cDev) {}

    /*************************************************************************************
     * Main Functionality
     *************************************************************************************/
    RetType init() {
        RESUME();

        uint8_t chipID = 0;
        this->device.dummy_byte = 0;

        I2CAddr_t addr = {
                .dev_addr = static_cast<uint16_t>((*reinterpret_cast<uint8_t *>(&device)) << 1),
                .mem_addr = 0x00, // Try reading Chip ID
                .mem_addr_size = 0x00000001U,
        };

        RetType ret = mI2C->read(addr, &chipID, 1);
        if (ret != RET_SUCCESS) return ret;
        SLEEP(INTERRUPT_WAIT_TIME);

        if (chipID != BMP390_CHIP_ID) return RET_ERROR;
        this->device.chip_id = chipID;

        ret = CALL(softReset());
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(getCalibrationData());
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getSensorData(uint8_t sensorComp) {
        RESUME();

        uint8_t regData[BMP3_LEN_P_T_DATA] = {0};
        struct bmp3_uncomp_data uncompensatedData = {0};

        RetType ret = CALL(getRegister(BMP3_REG_DATA, regData, BMP3_LEN_P_T_DATA));
        if (ret != RET_SUCCESS) return ret;

        SLEEP(INTERRUPT_WAIT_TIME);

        parseSensorData(regData, &uncompensatedData);
        compensateData(&uncompensatedData, &this->device.calib_data);

        RESET();
        return RET_SUCCESS;
    }


    RetType softReset() {
        RESUME();

        uint8_t cmdReadyStatus;
        uint8_t cmdErrorStatus;

        RetType ret = CALL(getRegister(BMP3_REG_SENS_STATUS, &cmdReadyStatus, 1));
        if (ret != RET_SUCCESS) return ret;
        SLEEP(INTERRUPT_WAIT_TIME);

        if ((cmdReadyStatus & BMP3_CMD_RDY)) {
            setRegister(reinterpret_cast<uint8_t *>(BMP3_REG_CMD), reinterpret_cast<const uint8_t *>(BMP3_SOFT_RESET),
                        1);
            SLEEP(2000);
        }

        RESET();
        return RET_SUCCESS;
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
    bmp3_data data;
    bmp3_settings settings;
    uint8_t chipID;
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;
    SPIDevice *mSPI;

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

    RetType getCalibrationData() {
        RESUME();

        uint8_t calibrationData[BMP3_LEN_CALIB_DATA] = {};

        RetType ret = getRegister(BMP3_REG_CALIB_DATA, calibrationData, BMP3_LEN_CALIB_DATA);
        if (ret != RET_SUCCESS) return ret;

        SLEEP(INTERRUPT_WAIT_TIME);

        parseCalibrationData(calibrationData);

        RESET();
        return RET_SUCCESS;
    }


    RetType setRegister(uint8_t *regAddress, const uint8_t *regData, uint32_t len) {
        RESUME();

        uint8_t temporaryBuffer[len * 2];
        uint8_t regAddrCount;
        size_t temporaryLen = len;

        temporaryBuffer[0] = regData[0];

        if (len > 1) {
            interleaveRegAddr(regAddress, temporaryBuffer, regData, len);
            temporaryLen = len * 2;
        }

        this->i2cAddr.mem_addr = *regAddress;


        RetType ret = CALL(mI2C->write(this->i2cAddr, temporaryBuffer, temporaryLen));
        if (ret != RET_SUCCESS) return ret;
        SLEEP(INTERRUPT_WAIT_TIME);

        RESET();
        return RET_SUCCESS;
    }

    void interleaveRegAddr(const uint8_t *regAddr, uint8_t *buff, const uint8_t *regData, uint32_t len) {
        uint32_t index;

        for (index = 1; index < len; index++) {
            buff[(index * 2) - 1] = regAddr[index];
            buff[index * 2] = regData[index];
        }
    }


    RetType getRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();
        this->i2cAddr.mem_addr = regAddress;

        RetType ret = mI2C->write(this->i2cAddr, regData, len);
        if (ret != RET_SUCCESS) return ret;
        SLEEP(INTERRUPT_WAIT_TIME);

        RESET();
        return RET_SUCCESS;
    }

    void parseCalibrationData(uint8_t *calibrationData) {
        struct bmp3_reg_calib_data *reg_calib_data = &this->device.calib_data.reg_calib_data;
        struct bmp3_quantized_calib_data *quantized_calib_data = &this->device.calib_data.quantized_calib_data;

        double temp;

        /* 1 / 2^8 */
        temp = 0.00390625f;
        reg_calib_data->par_t1 = BMP3_CONCAT_BYTES(calibrationData[1], calibrationData[0]);
        quantized_calib_data->par_t1 = ((double) reg_calib_data->par_t1 / temp);
        reg_calib_data->par_t2 = BMP3_CONCAT_BYTES(calibrationData[3], calibrationData[2]);

        temp = 1073741824.0f;
        quantized_calib_data->par_t2 = ((double) reg_calib_data->par_t2 / temp);
        reg_calib_data->par_t3 = (int8_t) calibrationData[4];

        temp = 281474976710656.0f;
        quantized_calib_data->par_t3 = ((double) reg_calib_data->par_t3 / temp);
        reg_calib_data->par_p1 = (int16_t) BMP3_CONCAT_BYTES(calibrationData[6], calibrationData[5]);

        temp = 1048576.0f;
        quantized_calib_data->par_p1 =
                ((double) (reg_calib_data->par_p1 - (16384)) / temp);
        reg_calib_data->par_p2 =
                (int16_t) BMP3_CONCAT_BYTES(calibrationData[8], calibrationData[7]);

        temp = 536870912.0f;
        quantized_calib_data->par_p2 =
                ((double) (reg_calib_data->par_p2 - (16384)) / temp);
        reg_calib_data->par_p3 = (int8_t) calibrationData[9];

        temp = 4294967296.0f;
        quantized_calib_data->par_p3 = ((double) reg_calib_data->par_p3 / temp);
        reg_calib_data->par_p4 = (int8_t) calibrationData[10];

        temp = 137438953472.0f;
        quantized_calib_data->par_p4 = ((double) reg_calib_data->par_p4 / temp);
        reg_calib_data->par_p5 = BMP3_CONCAT_BYTES(calibrationData[12], calibrationData[11]);

        /* 1 / 2^3 */
        temp = 0.125f;
        quantized_calib_data->par_p5 = ((double) reg_calib_data->par_p5 / temp);
        reg_calib_data->par_p6 = BMP3_CONCAT_BYTES(calibrationData[14], calibrationData[13]);

        temp = 64.0f;
        quantized_calib_data->par_p6 = ((double) reg_calib_data->par_p6 / temp);
        reg_calib_data->par_p7 = (int8_t) calibrationData[15];

        temp = 256.0f;
        quantized_calib_data->par_p7 = ((double) reg_calib_data->par_p7 / temp);
        reg_calib_data->par_p8 = (int8_t) calibrationData[16];

        temp = 32768.0f;
        quantized_calib_data->par_p8 = ((double) reg_calib_data->par_p8 / temp);
        reg_calib_data->par_p9 =
                (int16_t) BMP3_CONCAT_BYTES(calibrationData[18], calibrationData[17]);

        temp = 281474976710656.0f;
        quantized_calib_data->par_p9 = ((double) reg_calib_data->par_p9 / temp);
        reg_calib_data->par_p10 = (int8_t) calibrationData[19];

        temp = 281474976710656.0f;
        quantized_calib_data->par_p10 =
                ((double) reg_calib_data->par_p10 / temp);
        reg_calib_data->par_p11 = (int8_t) calibrationData[20];

        temp = 36893488147419103232.0f;
        quantized_calib_data->par_p11 =
                ((double) reg_calib_data->par_p11 / temp);
    }


    void parseSensorData(const uint8_t *reg_data, struct bmp3_uncomp_data *uncomp_data) {
        uint32_t data_xlsb;
        uint32_t data_lsb;
        uint32_t data_msb;

        data_xlsb = static_cast<uint32_t>(reg_data[0]);
        data_lsb = static_cast<uint32_t>(reg_data[1]) << 8;
        data_msb = static_cast<uint32_t>(reg_data[2]) << 16;
        uncomp_data->pressure = data_msb | data_lsb | data_xlsb;

        data_xlsb = static_cast<uint32_t>(reg_data[2]) << 16;
        (reg_data[3]);
        data_lsb = static_cast<uint32_t>(reg_data[2]) << 16;
        (reg_data[4]) << 8;
        data_msb = static_cast<uint32_t>(reg_data[2]) << 16;
        (reg_data[5]) << 16;
        uncomp_data->temperature = data_msb | data_lsb | data_xlsb;
    }

    void compensateData(bmp3_uncomp_data *uncompData, bmp3_calib_data *calibrationData) {
        compensateTemperature(&this->data.temperature, uncompData, calibrationData);
        compensatePressure(&this->data.pressure, uncompData, calibrationData);
    }


    void compensatePressure(double *pressure, const struct bmp3_uncomp_data *uncompData,
                            const struct bmp3_calib_data *calibrationData) {
        const struct bmp3_quantized_calib_data *quantizedCalibData = &calibrationData->quantized_calib_data;

        double comp_press;

        double tempData1;
        double tempData2;
        double tempData3;
        double tempData4;
        double tempData5;
        double tempData6;

        tempData1 = quantizedCalibData->par_p6 * quantizedCalibData->t_lin;
        tempData2 = quantizedCalibData->par_p7 * pow(quantizedCalibData->t_lin, 2);
        tempData3 = quantizedCalibData->par_p8 * pow(quantizedCalibData->t_lin, 3);
        tempData5 = quantizedCalibData->par_p5 + tempData1 + tempData2 + tempData3;
        tempData1 = quantizedCalibData->par_p2 * quantizedCalibData->t_lin;
        tempData2 = quantizedCalibData->par_p3 * pow(quantizedCalibData->t_lin, 2);
        tempData3 = quantizedCalibData->par_p4 * pow(quantizedCalibData->t_lin, 3);
        tempData6 = uncompData->pressure * (quantizedCalibData->par_p1 + tempData1 +
                                            tempData2 + tempData3);
        tempData1 = pow((double) uncompData->pressure, 2);
        tempData2 = quantizedCalibData->par_p9 + quantizedCalibData->par_p10 * quantizedCalibData->t_lin;
        tempData3 = tempData1 * tempData2;
        tempData4 = tempData3 + pow(static_cast<double>(uncompData->pressure), 3) *
                                quantizedCalibData->par_p11;
        comp_press = tempData5 + tempData6 + tempData4;

        if (comp_press < BMP3_MIN_PRES_DOUBLE) {
            comp_press = BMP3_MIN_PRES_DOUBLE;
        }

        if (comp_press > BMP3_MAX_PRES_DOUBLE) {
            comp_press = BMP3_MAX_PRES_DOUBLE;
        }

        (*pressure) = comp_press;

    }

    void compensateTemperature(double *temperature, const struct bmp3_uncomp_data *uncompData,
                               struct bmp3_calib_data *calibData) {
        int64_t uncompTemp = uncompData->temperature;
        double tempData1;
        double tempData2;

        tempData1 = static_cast<double>(uncompTemp - calibData->quantized_calib_data.par_t1);
        tempData2 = static_cast<double>(tempData1 * calibData->quantized_calib_data.par_t2);

        /* Update the compensated temperature in calib structure since this is
         * needed for pressure calculation */
        calibData->quantized_calib_data.t_lin =
                tempData2 + (tempData1 * tempData1) * calibData->quantized_calib_data.par_t3;

        /* Returns compensated temperature */
        if (calibData->quantized_calib_data.t_lin < BMP3_MIN_TEMP_DOUBLE) {
            calibData->quantized_calib_data.t_lin = BMP3_MIN_TEMP_DOUBLE;
        }

        if (calibData->quantized_calib_data.t_lin > BMP3_MAX_TEMP_DOUBLE) {
            calibData->quantized_calib_data.t_lin = BMP3_MAX_TEMP_DOUBLE;
        }

        (*temperature) = calibData->quantized_calib_data.t_lin;
    }

    float pow(double base, uint8_t power) {
        float pow_output = 1;

        while (power != 0) {
            pow_output = (float) base * pow_output;
            power--;
        }

        return pow_output;
    }

};


#endif //LAUNCH_CORE_BMP390_H
