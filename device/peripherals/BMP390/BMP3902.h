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
#include "device/StreamDevice.h"

#include "sched/macros/call.h"
#include "macros.h"


class BMP390 {
public:
    BMP390(I2CDevice &i2cDev, StreamDevice &uartDevice) : mI2C(&i2cDev), mUART(uartDevice) {}

    /*************************************************************************************
     * Main Functionality
     *************************************************************************************/
    RetType init() {
        RESUME();
        uint8_t chipID = 0;

        this->device.dummy_byte = 0;

        this->i2cAddr = {
                .dev_addr = BMP3_ADDR_I2C_SEC << 1,
                .mem_addr = BMP3_REG_CHIP_ID,
                .mem_addr_size = 1,
        };

        RetType ret = CALL(mI2C->read(this->i2cAddr, &this->device.chip_id, 1));
        if (this->device.chip_id != BMP390_CHIP_ID) return RET_ERROR;

        ret = CALL(softReset());
        if (ret == RET_ERROR) return ret;
        CALL(mUART.write((uint8_t *)"BMP390 Soft Reset successful\r\n", 28));

        ret = CALL(getCalibrationData());
        if (ret == RET_ERROR) return ret;
        CALL(mUART.write((uint8_t *)"BMP390 Calibration successful\r\n", 31));

        ret = CALL(initSettings());
        if (ret == RET_ERROR) return ret;
        CALL(mUART.write((uint8_t *)"BMP390 Settings Init successful\r\n", 33));
        RESET();
        return ret;
    }

    RetType getSensorData(double *pressure, double *temperature) {
        RESUME();

        static uint8_t regData[BMP3_LEN_P_T_DATA] = {0};
        static struct bmp3_uncomp_data uncompensatedData = {0};

        CALL(mUART.write((uint8_t *)"BMP390 Data Read called\r\n", 25));
        RetType ret = CALL(getRegister(BMP3_REG_DATA, regData, BMP3_LEN_P_T_DATA));
        if (ret != RET_SUCCESS) return ret;

        CALL(mUART.write((uint8_t *)"BMP390 Data Read successful\r\n", 29));

        parseSensorData(regData, &uncompensatedData);
        compensateData(&uncompensatedData, &this->device.calib_data);

        *pressure = this->data.pressure;
        *temperature = this->data.temperature;

        RESET();
        return RET_SUCCESS;
    }


    RetType softReset() {
        RESUME();

        static uint8_t cmdReadyStatus;
        static uint8_t cmdErrorStatus;
        static uint8_t regAddr = BMP3_REG_CMD;
        static uint8_t resetCmd = BMP3_SOFT_RESET;

        RetType ret = CALL(getRegister(BMP3_REG_SENS_STATUS, &cmdReadyStatus, 1));
        if (ret != RET_SUCCESS) return ret;

        if ((cmdReadyStatus & BMP3_CMD_RDY)) {
            ret = CALL(setRegister(&regAddr, &resetCmd, 1));
            if (ret != RET_SUCCESS) return ret;

            SLEEP(2000);
            ret = CALL(getRegister(BMP3_REG_ERR, &cmdErrorStatus, 1));
            if (ret != RET_SUCCESS) return ret;

        }

        RESET();
        return ret;
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

        RetType ret = CALL(getSensorStatus(status));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(getIntStatus(status));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(getErrStatus(status));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }

    RetType getSensorStatus(struct bmp3_status *status) {
        RESUME();

        uint8_t regAddr = BMP3_REG_SENS_STATUS;
        uint8_t regData;
        RetType ret = CALL(getRegister(regAddr, &regData, 1));
        if (ret != RET_SUCCESS) return ret;


        status->sensor.cmd_rdy = BMP3_GET_BITS(regData, BMP3_STATUS_CMD_RDY);
        status->sensor.drdy_press = BMP3_GET_BITS(regData, BMP3_STATUS_DRDY_PRESS);
        status->sensor.drdy_temp = BMP3_GET_BITS(regData, BMP3_STATUS_DRDY_TEMP);

        regAddr = BMP3_REG_EVENT;
        ret = CALL(getRegister(regAddr, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        status->pwr_on_rst = regData & 0x01;

        RESET();
        return ret;
    }

    RetType getIntStatus(struct bmp3_status *status) {
        RESUME();

        uint8_t regData;

        RetType ret = CALL(getRegister(BMP3_REG_INT_STATUS, &regData, 1));
        if (ret != RET_SUCCESS) return ret;


        status->intr.fifo_wm = BMP3_GET_BITS_POS_0(regData, BMP3_INT_STATUS_FWTM);
        status->intr.fifo_full = BMP3_GET_BITS(regData, BMP3_INT_STATUS_FFULL);
        status->intr.drdy = BMP3_GET_BITS(regData, BMP3_INT_STATUS_DRDY);


        RESET();
        return ret;
    }

    RetType getErrStatus(struct bmp3_status *status) {
        RESUME();

        uint8_t regData;

        RetType ret = CALL(getRegister(BMP3_REG_ERR, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        status->err.fatal = BMP3_GET_BITS_POS_0(regData, BMP3_ERR_FATAL);
        status->err.cmd = BMP3_GET_BITS(regData, BMP3_ERR_CMD);
        status->err.conf = BMP3_GET_BITS(regData, BMP3_ERR_CONF);

        RESET();
        return ret;
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

    RetType setSensorSettings(uint32_t desiredSettings) {
        RESUME();

        RetType ret = CALL(setPowerControl(desiredSettings));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setODRFilter(desiredSettings));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setIntCtrl(desiredSettings));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setAdvSettings(desiredSettings));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }

    RetType setOperatingMode() {
        RESUME();

        uint8_t lastSetMode;
        uint8_t currMode = this->settings.op_mode;

        RetType ret = CALL(getOperatingMode(&lastSetMode));
        if (ret != RET_SUCCESS) return ret;

        if (lastSetMode != BMP3_MODE_SLEEP) {
            ret = CALL(sleep());
            SLEEP(5000);
        }

        if (currMode == BMP3_MODE_NORMAL) {
            ret = CALL(setNormalMode());
        } else if (currMode == BMP3_MODE_FORCED) {
            ret = CALL(writePowerMode());
        }

        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }

    RetType sleep() {
        RESUME();

        uint8_t regAddr = BMP3_REG_PWR_CTRL;
        uint8_t operatingMode;

        RetType ret = CALL(getRegister(regAddr, &operatingMode, 1));
        if (ret != RET_SUCCESS) return ret;

        operatingMode = operatingMode & (~BMP3_OP_MODE_MSK);
        ret = CALL(setRegister(&regAddr, &operatingMode, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType setNormalMode() {
        RESUME();

        if (validateOsrOdr() != RET_SUCCESS) return RET_ERROR;

        uint8_t configErrorStatus;
        RetType ret = writePowerMode();
        ret = getRegister(BMP3_REG_ERR, &configErrorStatus, 1);
        if (ret != RET_SUCCESS) return ret;


        if (configErrorStatus & BMP3_ERR_CMD) {
            ret = RET_ERROR;
        }

        RESET();
        return ret;
    }

    RetType writePowerMode() {
        RESUME();

        uint8_t tempOpMode;
        uint8_t regAddr = BMP3_REG_PWR_CTRL;
        uint8_t operatingMode = this->settings.op_mode;

        RetType ret = CALL(getRegister(regAddr, &tempOpMode, 1));
        if (ret != RET_SUCCESS) return ret;

        tempOpMode = BMP3_SET_BITS(tempOpMode, BMP3_OP_MODE, operatingMode);
        ret = CALL(setRegister(&regAddr, &tempOpMode, 1));
        if (ret != RET_SUCCESS) return ret;

        writePowerModeEnd:
        RESET();
        return RET_SUCCESS;

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
    bmp3_dev device = {};
    bmp3_data data = {};
    bmp3_settings settings;
    uint8_t chipID;
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;
    StreamDevice &mUART;



    RetType initSettings() {
        RESUME();

        this->settings = {
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

        static uint16_t settingsSel = BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN | BMP3_SEL_PRESS_OS |
                               BMP3_SEL_TEMP_OS | BMP3_SEL_ODR | BMP3_SEL_DRDY_EN;

        RetType ret = CALL(setSensorSettings(settingsSel));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setOperatingMode());
        if (ret != RET_SUCCESS) return ret;


        RESET();
        return RET_SUCCESS;
    }

    RetType getCalibrationData() {
        RESUME();
        static uint8_t calibrationData[BMP3_LEN_CALIB_DATA] = {}; // Must be static! Hard fault from invalid memory access otherwise

        RetType ret = CALL(getRegister(BMP3_REG_CALIB_DATA, calibrationData, BMP3_LEN_CALIB_DATA));
        if (ret != RET_SUCCESS) return ret;

        parseCalibrationData(calibrationData);

        RESET();
        return ret;
    }


    RetType setRegister(uint8_t *regAddress, const uint8_t *regData, uint32_t len) {
        RESUME();
        CALL(mUART.write((uint8_t *)"Set Register Called\r\n", 23));

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
        CALL(mUART.write((uint8_t *)"Set Register Returned\r\n", 23));

        RESET();
        return ret;
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

        CALL(mUART.write((uint8_t *)"Get Register Called\r\n", 21));
        RetType ret = CALL(mI2C->read(this->i2cAddr, regData, len));
        if (ret != RET_SUCCESS) return ret;

        CALL(mUART.write((uint8_t *)"Get Register Returned\r\n", 23));

        RESET();
        return ret;
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

        data_xlsb = static_cast<uint32_t>(reg_data[3]);
        data_lsb = static_cast<uint32_t>(reg_data[4]) << 8;
        data_msb = static_cast<uint32_t>(reg_data[5]) << 16;
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

    RetType setPowerControl(uint32_t desiredSettings) {
        RESUME();

        uint8_t regAddr = BMP3_REG_PWR_CTRL;
        uint8_t regData = 0;

        RetType ret = CALL(getRegister(regAddr, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        if (desiredSettings & BMP3_SEL_PRESS_EN) {
            regData = BMP3_SET_BITS_POS_0(regData, BMP3_PRESS_EN, this->settings.press_en);
        }

        if (desiredSettings & BMP3_SEL_TEMP_EN) {
            regData = BMP3_SET_BITS(regData, BMP3_TEMP_EN, this->settings.temp_en);
        }

        ret = CALL(setRegister(&regAddr, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }

    RetType setODRFilter(uint32_t desiredSettings) {
        RESUME();

        uint8_t len = 0;
        uint8_t regAddr[3] = {0};
        uint8_t regData[4];
        struct bmp3_odr_filter_settings osrSettings = settings.odr_filter;

        RetType ret = CALL(getRegister(BMP3_REG_OSR, regData, 4));
        if (ret != RET_SUCCESS) return ret;

        if (areSettingsChanged((BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS), desiredSettings)) {
            if (desiredSettings & (BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS)) {
                if (desiredSettings & BMP3_SEL_PRESS_OS) {
                    regData[len] = BMP3_SET_BITS_POS_0(regData[0], BMP3_PRESS_OS, osrSettings.press_os);
                }

                if (desiredSettings & BMP3_SEL_TEMP_OS) {
                    regData[len] = BMP3_SET_BITS(regData[0], BMP3_TEMP_OS, osrSettings.temp_os);
                }

                regAddr[len] = BMP3_REG_OSR;
                len++;
            }
        }

        if (areSettingsChanged(BMP3_SEL_ODR, desiredSettings)) {
            if (osrSettings.odr > BMP3_ODR_0_001_HZ) {
                osrSettings.odr = BMP3_ODR_0_001_HZ;
            }

            regData[len] = BMP3_SET_BITS_POS_0(regData[1], BMP3_ODR, osrSettings.odr);

            regAddr[len] = BMP3_REG_ODR;
            len++;
        }

        if (areSettingsChanged(BMP3_SEL_IIR_FILTER, desiredSettings)) {
            regData[len] = BMP3_SET_BITS(regData[3], BMP3_IIR_FILTER, osrSettings.iir_filter);

            regAddr[len] = BMP3_REG_CONFIG;
            len++;
        }

        if (settings.op_mode == BMP3_MODE_NORMAL) {
            ret = validateOsrOdr() == TRUE ? RET_SUCCESS : RET_ERROR;
            if (ret != RET_SUCCESS) return ret;
        }

        ret = CALL(setRegister(regAddr, regData, len));
        if (ret != RET_SUCCESS) return ret;

         RESET();
        return ret;
    }

    RetType setIntCtrl(uint32_t desiredSettings) {
        RESUME();

        uint8_t regAddr = BMP3_REG_INT_CTRL;
        uint8_t regData;
        bmp3_int_ctrl_settings intSettings = this->settings.int_settings;

        RetType ret = CALL(getRegister(regAddr, &regData, 1));
        if (ret != RET_SUCCESS) return ret;


        if (desiredSettings & BMP3_SEL_OUTPUT_MODE) {
            regData = BMP3_SET_BITS_POS_0(regData, BMP3_INT_OUTPUT_MODE, intSettings.output_mode);
        }

        if (desiredSettings & BMP3_SEL_LEVEL) {
            regData = BMP3_SET_BITS(regData, BMP3_INT_LEVEL, intSettings.level);
        }

        if (desiredSettings & BMP3_SEL_LATCH) {
            regData =
                    BMP3_SET_BITS(regData, BMP3_INT_LATCH, intSettings.latch);
        }

        if (desiredSettings & BMP3_SEL_DRDY_EN) {
            regData = BMP3_SET_BITS(regData, BMP3_INT_DRDY_EN, intSettings.drdy_en);
        }

        ret = CALL(setRegister(&regAddr, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        setIntCtrlEnd:
    RESET();
        return ret;
    }

    RetType setAdvSettings(uint32_t desiredSettings) {
        RESUME();

        bmp3_adv_settings advSettings = this->settings.adv_settings;
        uint8_t regAddr = BMP3_REG_IF_CONF;
        uint8_t regData = 0;

        RetType ret = CALL(getRegister(regAddr, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        if (desiredSettings & BMP3_SEL_I2C_WDT_EN) {
            regData = BMP3_SET_BITS(regData, BMP3_I2C_WDT_EN, advSettings.i2c_wdt_en);
        }

        if (desiredSettings & BMP3_SEL_I2C_WDT) {
            regData = BMP3_SET_BITS(regData, BMP3_I2C_WDT_SEL, advSettings.i2c_wdt_sel);
        }

        ret = CALL(setRegister(&regAddr, &regData, 1));
        if (ret != RET_SUCCESS) return ret;


        RESET();
        return ret;
    }

    RetType getOperatingMode(uint8_t *opMode) {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_PWR_CTRL, opMode, 1));
        if (ret != RET_SUCCESS) return ret;

        *opMode = BMP3_GET_BITS(*opMode, BMP3_OP_MODE);

        RESET();
        return ret;
    }

    float pow(double base, uint8_t power) {
        float pow_output = 1;

        while (power != 0) {
            pow_output = (float) base * pow_output;
            power--;
        }

        return pow_output;
    }

    bool areSettingsChanged(uint32_t subSettings, uint32_t desiredSettings) {
        if ((subSettings & desiredSettings)) {
            return true;
        } else {
            return false;
        }
    }

    uint32_t calculatePressMeasTime() {
#ifdef BMP3_FLOAT_COMPENSATION
        double base = 2.0;
        float partialOut;
#else
        uint8_t base = 2;
        uint32_t partial_out;
#endif /* BMP3_FLOAT_COMPENSATION */
        partialOut = pow(base, settings.odr_filter.press_os);
        return static_cast<uint32_t>(BMP3_SETTLE_TIME_PRESS + partialOut * BMP3_ADC_CONV_TIME);
    }

    uint32_t calculateTempMeasTime() {
#ifdef BMP3_FLOAT_COMPENSATION
        double base = 2.0;
        float partialOut;
#else
        uint8_t base = 2;
            uint32_t partial_out;
#endif /* BMP3_FLOAT_COMPENSATION */
        partialOut = pow(base, settings.odr_filter.temp_os);
        return static_cast<uint32_t>(BMP3_SETTLE_TIME_TEMP + partialOut * BMP3_ADC_CONV_TIME);
    }

    bool validateOsrOdr() {
        uint32_t measT = 234;
        uint32_t measTP = 0;

        /* Sampling period corresponding to ODR in microseconds  */
        uint32_t odr[18] = {5000, 10000, 20000, 40000, 80000,
                            160000, 320000, 640000, 1280000, 2560000,
                            5120000, 10240000, 20480000, 40960000, 81920000,
                            163840000, 327680000, 655360000};

        if (settings.press_en) {
            /* Calculate the pressure measurement duration */
            measTP += calculatePressMeasTime();
        }

        if (settings.temp_en) {
            /* Calculate the temperature measurement duration */
            measTP += calculateTempMeasTime();
        }

        /* Constant 234us added to the summation of temperature and pressure
         * measurement duration */
        measT += measTP;

        return measT < odr[settings.odr_filter.odr];
    }
};


#endif //LAUNCH_CORE_BMP390_H
