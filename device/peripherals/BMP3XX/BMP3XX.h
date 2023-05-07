/**
 * Facade for the BMP3 API that utilizes the scheduler
 *
 * TODO: Update remaining functions. Mostly initialization and sensor readings implemented only for now
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_BMP3XX_H
#define LAUNCH_CORE_BMP3XX_H
#define TMP117_DATA_STRUCT(variable_name) SHTC3_DATA_T variable_name = {.id = 10001, .pressure = 0, temperature = 0}

#include "device/peripherals/BMP3XX/bmp3_defs.h"
#include "return.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "device/SPIDevice.h"
#include "device/I2CDevice.h"

#include "sched/macros/call.h"
#include "macros.h"

using BMP3XX_DATA_T = struct {
    uint16_t id;
    double pressure;
    double temp;
};

class BMP3XX {
public:
    BMP3XX(I2CDevice &i2cDev) : mI2C(&i2cDev) {}

    /*************************************************************************************
     * Main Functionality
     *************************************************************************************/
    RetType init(bool is388 = true) {
        RESUME();
        this->device.dummy_byte = 0;

        this->i2cAddr = {
                .dev_addr = BMP3_ADDR_I2C_SEC << 1,
                .mem_addr = BMP3_REG_CHIP_ID,
                .mem_addr_size = 1,
        };

        RetType ret = CALL(mI2C->read(this->i2cAddr, &this->device.chip_id, 1, 50));
        if (this->device.chip_id != (is388 ? BMP3_CHIP_ID : BMP390_CHIP_ID)) return RET_ERROR;

        ret = CALL(softReset());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(getCalibrationData());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(initSettings());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return ret;
    }

    RetType getPressureAndTemp(double *pressure, double *temperature) {
        RESUME();

        static uint8_t regData[BMP3_LEN_P_T_DATA] = {0};
        static struct bmp3_uncomp_data uncompensatedData = {0};

        RetType ret = CALL(getRegister(BMP3_REG_DATA, regData, BMP3_LEN_P_T_DATA));
        if (ret != RET_SUCCESS) return ret;

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

        RetType ret = CALL(getRegister(BMP3_REG_SENS_STATUS, &cmdReadyStatus, 1));
        if (ret != RET_SUCCESS) return ret;

        if ((cmdReadyStatus & BMP3_CMD_RDY)) {
            ret = CALL(setRegister(BMP3_REG_CMD, reinterpret_cast<uint8_t *>(BMP3_SOFT_RESET), 1));
            if (ret != RET_SUCCESS) return ret;

            SLEEP(2000);
            ret = CALL(getRegister(BMP3_REG_ERR, &cmdErrorStatus, 1));
            if (ret != RET_SUCCESS) return ret;

            if (cmdErrorStatus & BMP3_REG_CMD) return RET_ERROR;
        }

        RESET();
        return ret;
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

        static uint8_t regData;
        RetType ret = CALL(getRegister(BMP3_REG_SENS_STATUS, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        status->sensor.cmd_rdy = BMP3_GET_BITS(regData, BMP3_STATUS_CMD_RDY);
        status->sensor.drdy_press = BMP3_GET_BITS(regData, BMP3_STATUS_DRDY_PRESS);
        status->sensor.drdy_temp = BMP3_GET_BITS(regData, BMP3_STATUS_DRDY_TEMP);

        ret = CALL(getRegister(BMP3_REG_EVENT, &regData, 1));
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

    /*************************************************************************************
     * Settings
     *************************************************************************************/
    RetType setSensorSettings(uint32_t desiredSettings) {
        RESUME();

        RetType ret;

        // Power Control Settings
        if (areSettingsChanged(BMP3_POWER_CNTL, desiredSettings)) {
            ret = CALL(setPowerControl(desiredSettings));
            if (ret != RET_SUCCESS) return ret;
        }

        // Oversampling, ODR and Filter Settings
        if (areSettingsChanged(BMP3_ODR_FILTER, desiredSettings)) {
            ret = CALL(setODRFilter(desiredSettings)); // TODO: Might be causing issues
            if (ret != RET_SUCCESS) return ret;
        }

        // Interrupt Control Settings
        if (areSettingsChanged(BMP3_INT_CTRL, desiredSettings)) {
            ret = CALL(setIntCtrl(desiredSettings));
            if (ret != RET_SUCCESS) return ret;
        }

        // Advanced Settings
        if (areSettingsChanged(BMP3_ADV_SETT, desiredSettings)) {
            ret = CALL(setAdvSettings(desiredSettings));
            if (ret != RET_SUCCESS) return ret;
        }

        RESET();
        return ret;
    }

    RetType setOperatingMode() {
        RESUME();

        static uint8_t lastSetMode;
        static uint8_t currMode = this->settings.op_mode;

        RetType ret = CALL(getOperatingMode(&lastSetMode));
        if (ret != RET_SUCCESS) return ret;


        if (lastSetMode != BMP3_MODE_SLEEP) {
            ret = CALL(sleep());
            if (ret != RET_SUCCESS) return ret;

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

        static uint8_t operatingMode;

        RetType ret = CALL(getRegister(BMP3_REG_PWR_CTRL, &operatingMode, 1));
        if (ret != RET_SUCCESS) return ret;

        operatingMode = operatingMode & (~(BMP3_OP_MODE_MSK));

        ret = CALL(setRegister(BMP3_REG_PWR_CTRL, &operatingMode, 1));

        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType setNormalMode() {
        RESUME();
        static uint8_t configErrorStatus;

        RetType ret = CALL(validateNormalModeSettings());
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(writePowerMode());
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(getRegister(BMP3_REG_ERR, &configErrorStatus, 1));
        if (ret != RET_SUCCESS) return ret;

        if (configErrorStatus & BMP3_ERR_CMD) {
            ret = RET_ERROR;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType validateNormalModeSettings() {
        RESUME();

        RetType ret = CALL(getODRFilterSettings());
        if (ret != RET_SUCCESS) return ret;
        if (!validateOsrOdr()) return RET_ERROR;

        RESET();
        return RET_SUCCESS;
    }

    RetType writePowerMode() {
        RESUME();

        static uint8_t tempOpMode;

        RetType ret = CALL(getRegister(BMP3_REG_PWR_CTRL, &tempOpMode, 1));
        if (ret != RET_SUCCESS) return ret;

        tempOpMode = BMP3_SET_BITS(tempOpMode, BMP3_OP_MODE, settings.op_mode);
        ret = CALL(setRegister(BMP3_REG_PWR_CTRL, &tempOpMode, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;

    }

    RetType getODRFilterSettings() {
        RESUME();

        static uint8_t regData[4];
        RetType ret = CALL(getRegister(BMP3_REG_OSR, regData, 4));
        if (ret != RET_SUCCESS) return ret;

        parseODRFilterSettings(regData);

        RESET();
        return RET_SUCCESS;
    }

private:
    bmp3_dev device = {};
    bmp3_data data = {};
    bmp3_settings settings;
    uint8_t chipID;
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;


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


        RetType ret = CALL(setSensorSettings(
                BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN | BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS | BMP3_SEL_ODR |
                BMP3_SEL_DRDY_EN));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(setOperatingMode());
        if (ret != RET_SUCCESS) return ret;


        RESET();
        return RET_SUCCESS;
    }

    RetType getCalibrationData() {
        RESUME();
        static uint8_t calibrationData[BMP3_LEN_CALIB_DATA] = {};

        RetType ret = CALL(getRegister(BMP3_REG_CALIB_DATA, calibrationData, BMP3_LEN_CALIB_DATA));
        if (ret != RET_SUCCESS) return ret;

        parseCalibrationData(calibrationData);

        RESET();
        return ret;
    }

    RetType setRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();


        this->i2cAddr.mem_addr = regAddress;

        RetType ret = CALL(mI2C->write(this->i2cAddr, regData, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }


    RetType setRegister(uint8_t const *regAddress, const uint8_t *regData, uint32_t len) {
        RESUME();

        uint8_t temporaryBuffer[len * 2];
        size_t temporaryLen = len;

        temporaryBuffer[0] = regData[0];

        if (len > 1) {
            interleaveRegAddr(regAddress, temporaryBuffer, regData, len);
            temporaryLen = len * 2;
        }

        this->i2cAddr.mem_addr = *regAddress;


        RetType ret = CALL(mI2C->write(this->i2cAddr, temporaryBuffer, temporaryLen));
        if (ret != RET_SUCCESS) return ret;

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

        RetType ret = CALL(mI2C->read(this->i2cAddr, regData, len));
        if (ret != RET_SUCCESS) return ret;

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

        tempData1 = uncompTemp - calibData->quantized_calib_data.par_t1;
        tempData2 = tempData1 * calibData->quantized_calib_data.par_t2;

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

        static uint8_t regData = 0;

        RetType ret = CALL(getRegister(BMP3_REG_PWR_CTRL, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        if (desiredSettings & BMP3_SEL_PRESS_EN) {
            regData = BMP3_SET_BITS_POS_0(regData, BMP3_PRESS_EN, this->settings.press_en);
        }

        if (desiredSettings & BMP3_SEL_TEMP_EN) {
            regData = BMP3_SET_BITS(regData, BMP3_TEMP_EN, this->settings.temp_en);
        }

        ret = CALL(setRegister(BMP3_REG_PWR_CTRL, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }

    RetType setODRFilter(uint32_t desiredSettings) {
        RESUME();

        static uint8_t len = 0;
        static uint8_t regAddr[3] = {0};
        static uint8_t regData[4];

        RetType ret = CALL(getRegister(BMP3_REG_OSR, regData, 4));
        if (ret != RET_SUCCESS) return ret;

        // OSR Data
        if (areSettingsChanged((BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS), desiredSettings)) {
            fillOSRData(desiredSettings, regAddr, regData, &len);
        }

        // ODR Data
        if (areSettingsChanged(BMP3_SEL_ODR, desiredSettings)) {
            fillOdrData(regAddr, regData, &len);
        }

        // Filter Data
        if (areSettingsChanged(BMP3_SEL_IIR_FILTER, desiredSettings)) {
            fillFilterData(regAddr, regData, &len);
        }

        // OSR ODR Validation
        if (settings.op_mode == BMP3_MODE_NORMAL) {
            ret = validateOsrOdr() ? RET_SUCCESS : RET_ERROR;
            if (ret != RET_SUCCESS) return ret;
        }

        // Burst Write
        ret = CALL(setRegister(regAddr, regData, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }

    void fillOSRData(uint32_t desiredSettings, uint8_t *addr, uint8_t *regData, uint8_t *len) {
        if (desiredSettings & (BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS)) {
            /* Pressure over sampling settings check */
            if (desiredSettings & BMP3_SEL_PRESS_OS) {
                /* Set the pressure over sampling settings in the
                 * register variable */
                regData[*len] = BMP3_SET_BITS_POS_0(regData[0], BMP3_PRESS_OS,
                                                    settings.odr_filter.press_os);
            }

            /* Temperature over sampling settings check */
            if (desiredSettings & BMP3_SEL_TEMP_OS) {
                /* Set the temperature over sampling settings in the
                 * register variable */
                regData[*len] = BMP3_SET_BITS(regData[0], BMP3_TEMP_OS, settings.odr_filter.temp_os);
            }

            /* 0x1C is the register address of over sampling register */
            addr[*len] = BMP3_REG_OSR;
            (*len)++;
        }
    }

    void fillOdrData(uint8_t *addr, uint8_t *regData, uint8_t *len) {
        /* Limit the ODR to 0.001525879 Hz*/
        if (settings.odr_filter.odr > BMP3_ODR_0_001_HZ) {
            settings.odr_filter.odr = BMP3_ODR_0_001_HZ;
        }

        /* Set the ODR settings in the register variable */
        regData[*len] =
                BMP3_SET_BITS_POS_0(regData[1], BMP3_ODR, settings.odr_filter.odr);

        /* 0x1D is the register address of output data rate register */
        addr[*len] = BMP3_REG_ODR;
        (*len)++;
    }


    void fillFilterData(uint8_t *addr, uint8_t *regData, uint8_t *len) {

        /* Set the iir settings in the register variable */
        regData[*len] = BMP3_SET_BITS(regData[3], BMP3_IIR_FILTER, settings.odr_filter.iir_filter);

        /* 0x1F is the register address of iir filter register */
        addr[*len] = BMP3_REG_CONFIG;
        (*len)++;
    }

    RetType setIntCtrl(uint32_t desiredSettings) {
        RESUME();

        static uint8_t regData;

        RetType ret = CALL(getRegister(BMP3_REG_INT_CTRL, &regData, 1));
        if (ret != RET_SUCCESS) return ret;


        if (desiredSettings & BMP3_SEL_OUTPUT_MODE) {
            regData = BMP3_SET_BITS_POS_0(regData, BMP3_INT_OUTPUT_MODE, settings.int_settings.output_mode);
        }

        if (desiredSettings & BMP3_SEL_LEVEL) {
            regData = BMP3_SET_BITS(regData, BMP3_INT_LEVEL, settings.int_settings.level);
        }

        if (desiredSettings & BMP3_SEL_LATCH) {
            regData = BMP3_SET_BITS(regData, BMP3_INT_LATCH, settings.int_settings.latch);
        }

        if (desiredSettings & BMP3_SEL_DRDY_EN) {
            regData = BMP3_SET_BITS(regData, BMP3_INT_DRDY_EN, settings.int_settings.drdy_en);
        }

        ret = CALL(setRegister(BMP3_REG_INT_CTRL, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }


    RetType setAdvSettings(uint32_t desiredSettings) {
        RESUME();

        static uint8_t regData = 0;

        RetType ret = CALL(getRegister(BMP3_REG_IF_CONF, &regData, 1));
        if (ret != RET_SUCCESS) return ret;

        if (desiredSettings & BMP3_SEL_I2C_WDT_EN) {
            regData = BMP3_SET_BITS(regData, BMP3_I2C_WDT_EN, settings.adv_settings.i2c_wdt_en);
        }

        if (desiredSettings & BMP3_SEL_I2C_WDT) {
            regData = BMP3_SET_BITS(regData, BMP3_I2C_WDT_SEL, settings.adv_settings.i2c_wdt_sel);
        }

        ret = CALL(setRegister(BMP3_REG_IF_CONF, &regData, 1));
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

    void parseSettings(const uint8_t *reg_data) {
        parseIntCtrlSettings(&reg_data[0]);
        parseAdvancedSettings(&reg_data[1]);
        parsePowerCtrlSettings(&reg_data[2]);
        parseODRFilterSettings(&reg_data[3]);
    }

    void parseIntCtrlSettings(const uint8_t *reg_data) {
        settings.int_settings.output_mode = BMP3_GET_BITS_POS_0(*reg_data, BMP3_INT_OUTPUT_MODE);
        settings.int_settings.level = BMP3_GET_BITS(*reg_data, BMP3_INT_LEVEL);
        settings.int_settings.latch = BMP3_GET_BITS(*reg_data, BMP3_INT_LATCH);
        settings.int_settings.drdy_en = BMP3_GET_BITS(*reg_data, BMP3_INT_DRDY_EN);
    }

    void parseAdvancedSettings(const uint8_t *reg_data) {
        settings.adv_settings.i2c_wdt_en = BMP3_GET_BITS(*reg_data, BMP3_I2C_WDT_EN);
        settings.adv_settings.i2c_wdt_sel = BMP3_GET_BITS(*reg_data, BMP3_I2C_WDT_SEL);
    }

    void parsePowerCtrlSettings(const uint8_t *reg_data) {
        settings.op_mode = BMP3_GET_BITS(*reg_data, BMP3_OP_MODE);
        settings.press_en = BMP3_GET_BITS_POS_0(*reg_data, BMP3_PRESS_EN);
        settings.temp_en = BMP3_GET_BITS(*reg_data, BMP3_TEMP_EN);
    }

    void parseODRFilterSettings(const uint8_t *regData) {
        uint8_t index = 0;

        settings.odr_filter.press_os = BMP3_GET_BITS_POS_0(regData[index], BMP3_PRESS_OS);
        settings.odr_filter.temp_os = BMP3_GET_BITS(regData[index], BMP3_TEMP_OS);

        index++;
        settings.odr_filter.odr = BMP3_GET_BITS_POS_0(regData[index], BMP3_ODR);

        index = index + 2;
        settings.odr_filter.iir_filter = BMP3_GET_BITS(regData[index], BMP3_IIR_FILTER);
    }
};



#endif //LAUNCH_CORE_BMP3XX_H
