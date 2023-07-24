/**
 * Platform Independent BMP388/BMP390 Driver
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_BMP3XX_H
#define LAUNCH_CORE_BMP3XX_H
#define BMP3XX_DATA_STRUCT(variable_name) BMP3XX_DATA_T variable_name = {.id = 10001, .pressure = 0, .temperature = 0}

#include <math.h>
#include <string.h>

#include "device/peripherals/BMP3XX/bmp3_defs.h"
#include "device/I2CDevice.h"
#include "return.h"
#include "macros.h"


using BMP3XX_DATA_T = struct {
    const uint16_t id;
    double pressure; // hPa
    double temperature; // C
};

class BMP3XX : public Device {
public:
    BMP3XX(I2CDevice &i2cDev, const uint16_t address = BMP3_ADDR_I2C_PRIM, const char *name = "BMP3XX")
            : Device(name), mI2C(&i2cDev),
              i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

    /*************************************************************************************
     * Main Functionality
     *************************************************************************************/
    RetType init() override {
        RESUME();

        RetType ret = CALL(checkChipID());
        ERROR_CHECK(ret);

        ret = CALL(softReset());
        ERROR_CHECK(ret);

        ret = CALL(getCalibrationData());
        ERROR_CHECK(ret);

        ret = CALL(initSettings());
        ERROR_CHECK(ret);

        RESET();
        return ret;
    }

    /**
     * @brief Wrapper for filling in BMP3XX_DATA_T struct
     * @param data - Pointer to BMP3XX_DATA_T struct
     * @return Scheduler Status
     */
    RetType getData(BMP3XX_DATA_T *data) {
        RESUME();
        RetType ret = CALL(getPressureAndTemp(&data->pressure, &data->temperature));

        RESET();
        return ret;
    }

    /**
     * @brief Retrieves sensor data
     * @param pressure - Pointer to pressure variable
     * @param temperature - Pointer to temperature variable
     * @return Scheduler Status
     */
    RetType getPressureAndTemp(double *pressure, double *temperature) {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_DATA, mBuff, BMP3_LEN_P_T_DATA));
        ERROR_CHECK(ret);

        struct bmp3_uncomp_data uncompensatedData = {0};
        parseSensorData(mBuff, &uncompensatedData);
        ret = compensateData(&uncompensatedData, &this->device.calib_data); // Bounds checked here
        if (RET_SUCCESS == ret) {
            *pressure = this->data.pressure;
            *temperature = this->data.temperature;
        }

        RESET();
        return ret;
    }


    /**
     * Get the current status of the device
     * @param status - Pointer to status struct
     * @return Scheduler Status
     */
    RetType getStatus(struct bmp3_status *status) {
        RESUME();

        RetType ret = CALL(getSensorStatus(status));
        ERROR_CHECK(ret);

        ret = CALL(getIntStatus(status));
        ERROR_CHECK(ret);

        ret = CALL(getErrStatus(status));
        ERROR_CHECK(ret);

        RESET();
        return ret;
    }

    /**
     * @brief Get the status of the sensor specifically
     * @param status - Pointer to status struct
     * @return Scheduler Status
     */
    RetType getSensorStatus(struct bmp3_status *status) {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_SENS_STATUS, mBuff, 1));
        ERROR_CHECK(ret);

        status->sensor.cmd_rdy = BMP3_GET_BITS(mBuff[0], BMP3_STATUS_CMD_RDY);
        status->sensor.drdy_press = BMP3_GET_BITS(mBuff[0], BMP3_STATUS_DRDY_PRESS);
        status->sensor.drdy_temp = BMP3_GET_BITS(mBuff[0], BMP3_STATUS_DRDY_TEMP);

        ret = CALL(getRegister(BMP3_REG_EVENT, mBuff, 1));
        ERROR_CHECK(ret);

        status->pwr_on_rst = mBuff[0] & 0x01;

        RESET();
        return ret;
    }

    /**
     * @brief Get the interrupt status of the device
     * @param status - Pointer to status struct
     * @return Scheduler Status
     */
    RetType getIntStatus(struct bmp3_status *status) {
        RESUME();


        RetType ret = CALL(getRegister(BMP3_REG_INT_STATUS, mBuff, 1));
        ERROR_CHECK(ret);


        status->intr.fifo_wm = BMP3_GET_BITS_POS_0(mBuff[0], BMP3_INT_STATUS_FWTM);
        status->intr.fifo_full = BMP3_GET_BITS(mBuff[0], BMP3_INT_STATUS_FFULL);
        status->intr.drdy = BMP3_GET_BITS(mBuff[0], BMP3_INT_STATUS_DRDY);


        RESET();
        return ret;
    }

    /**
     * Get the error status of the device
     * @param status - Pointer to status struct
     * @return Scheduler status
     */
    RetType getErrStatus(struct bmp3_status *status) {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_ERR, mBuff, 1));
        ERROR_CHECK(ret);

        status->err.fatal = BMP3_GET_BITS_POS_0(mBuff[0], BMP3_ERR_FATAL);
        status->err.cmd = BMP3_GET_BITS(mBuff[0], BMP3_ERR_CMD);
        status->err.conf = BMP3_GET_BITS(mBuff[0], BMP3_ERR_CONF);

        RESET();
        return ret;
    }

    /*************************************************************************************
     * Settings
     *************************************************************************************/

    /**
     * @brief Configure the sensor settings
     * @param desiredSettings - Desired settings to be configured
     * @return Scheduler Status
     */
    RetType setSensorSettings(uint32_t desiredSettings) {
        RESUME();

        RetType ret;

        // Power Control Settings
        if (areSettingsChanged(BMP3_POWER_CNTL, desiredSettings)) {
            ret = CALL(setPowerControl(desiredSettings));
            ERROR_CHECK(ret);
        }

        // Oversampling, ODR and Filter Settings
        if (areSettingsChanged(BMP3_ODR_FILTER, desiredSettings)) {
            ret = CALL(setODRFilter(desiredSettings)); // TODO: Might be causing issues
            ERROR_CHECK(ret);
        }

        // Interrupt Control Settings
        if (areSettingsChanged(BMP3_INT_CTRL, desiredSettings)) {
            ret = CALL(setIntCtrl(desiredSettings));
            ERROR_CHECK(ret);
        }

        // Advanced Settings
        if (areSettingsChanged(BMP3_ADV_SETT, desiredSettings)) {
            ret = CALL(setAdvSettings(desiredSettings));
            ERROR_CHECK(ret);
        }

        RESET();
        return ret;
    }

    /**
     * @brief Set mode of operation for the sensor
     * @return Scheduler Status
     */
    RetType setOperatingMode() {
        RESUME();

        RetType ret = CALL(getOperatingMode(mBuff));
        ERROR_CHECK(ret);

        if (BMP3_MODE_SLEEP != mBuff[0]) {
            ret = CALL(sleep());
            ERROR_CHECK(ret);

            SLEEP(5000);
        }

        if (BMP3_MODE_NORMAL == this->settings.op_mode) {
            ret = CALL(setNormalMode());
        } else if (BMP3_MODE_FORCED == this->settings.op_mode) {
            ret = CALL(writePowerMode());
        }

        RESET();
        return ret;
    }

    /**
     * @brief Put the device to sleep mode
     * @return Scheduler Status
     */
    RetType sleep() {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_PWR_CTRL, mBuff, 1));
        ERROR_CHECK(ret);

        mBuff[0] &= ~BMP3_OP_MODE_MSK;

        ret = CALL(setRegister(BMP3_REG_PWR_CTRL, mBuff, 1));

        RESET();
        return ret;
    }

    /**
     * @brief Put the device into its normal operating mode
     * @return Scheduler status
     */
    RetType setNormalMode() {
        RESUME();

        RetType ret = CALL(validateNormalModeSettings());
        ERROR_CHECK(ret);

        ret = CALL(writePowerMode());
        ERROR_CHECK(ret);

        ret = CALL(getRegister(BMP3_REG_ERR, mBuff, 1));
        ERROR_CHECK(ret);

        if (mBuff[0] & BMP3_ERR_CMD) ret = RET_ERROR;

        RESET();
        return ret;
    }

    /**
     * @brief Validate the settings of the device in normal mode
     * @return Scheduler status
     */
    RetType validateNormalModeSettings() {
        RESUME();

        RetType ret = CALL(getODRFilterSettings());
        ERROR_CHECK(ret);
        if (!validateOsrOdr()) ret = RET_ERROR;

        RESET();
        return ret;
    }

    /**
     * @brief Set the power mode of the device
     * @return Scheduler status
     */
    RetType writePowerMode() {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_PWR_CTRL, mBuff, 1));
        ERROR_CHECK(ret);

        mBuff[0] = BMP3_SET_BITS(mBuff[0], BMP3_OP_MODE, settings.op_mode);
        ret = CALL(setRegister(BMP3_REG_PWR_CTRL, mBuff, 1));

        RESET();
        return ret;

    }

    /**
     * @brief Retrieve the data rate filter settings
     * @return Scheduler status
     */
    RetType getODRFilterSettings() {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_OSR, mBuff, 4));
        if (RET_SUCCESS == ret) {
            parseODRFilterSettings(mBuff);
        }

        RESET();
        return ret;
    }

private:
    bmp3_dev device = {};
    bmp3_data data = {};
    bmp3_settings settings;
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;
    uint8_t mBuff[BMP3_LEN_CALIB_DATA]; // BMP3_LEN_CALIB_DATA is largest size

    /**
     * @brief Check if the chip ID is correct for the BMP388/BMP390
     * @return Scheduler status
     */
    RetType checkChipID() {
        RESUME();

        this->i2cAddr.mem_addr = BMP3_REG_CHIP_ID;
        RetType ret = CALL(getRegister(BMP3_REG_CHIP_ID, mBuff, 1));
        if ((BMP3_CHIP_ID != mBuff[0]) && (BMP390_CHIP_ID != mBuff[0])) {
            ret = RET_ERROR;
        }

        RESET();
        return ret;
    }


    /**
     * Initialize the initial settings for the device for basic operation
     * @return Scheduler Status
     */
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
        ERROR_CHECK(ret);

        ret = CALL(setOperatingMode());


        RESET();
        return ret;
    }

    /**
     * @brief Soft reset the BMP sensor
     * @return
     */
    RetType softReset() {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_SENS_STATUS, mBuff, 1));
        ERROR_CHECK(ret);

        if ((mBuff[0] & BMP3_CMD_RDY)) {
            mBuff[0] = BMP3_SOFT_RESET;
            ret = CALL(setRegister(BMP3_REG_CMD, mBuff, 1));
            ERROR_CHECK(ret);

            SLEEP(2000);
            ret = CALL(getRegister(BMP3_REG_ERR, mBuff, 1));
            ERROR_CHECK(ret);

            if (mBuff[0] & BMP3_REG_CMD) ret = RET_ERROR;
        }

        RESET();
        return ret;
    }

    /**
     * @brief Get the calibration values for sensor data
     * @return Scheduler status
     */
    RetType getCalibrationData() {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_CALIB_DATA, mBuff, BMP3_LEN_CALIB_DATA));
        ERROR_CHECK(ret);

        parseCalibrationData(mBuff);

        RESET();
        return ret;
    }

    /**
     * Set a single register in the device
     * @param regAddress - Address of the register to set
     * @param regData - Data to write to the register
     * @param len - Length of the data to write
     * @return
     */
    RetType setRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();


        this->i2cAddr.mem_addr = regAddress;

        RetType ret = CALL(mI2C->write(this->i2cAddr, regData, len));
        ERROR_CHECK(ret);

        RESET();
        return ret;
    }


    /**
     * @brief Set multiple registers in the device
     * @param regAddress - Address of the first register to set
     * @param regData - Data to write to the registers
     * @param len - Length of the data to write
     * @return scheduler status
     */
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
        ERROR_CHECK(ret);

        RESET();
        return ret;
    }

    /**
     * @brief Helper function to interleave the register addresses and data
     * @param regAddr - Array of register addresses
     * @param buff - Buffer to write interleaved data to
     * @param regData - Array of data to interleave
     * @param len - Length of the data to interleave
     */
    void interleaveRegAddr(const uint8_t *regAddr, uint8_t *buff, const uint8_t *regData, uint32_t len) {
        uint32_t index;

        for (index = 1; index < len; index++) {
            buff[(index * 2) - 1] = regAddr[index];
            buff[index * 2] = regData[index];
        }
    }

    /**
     * @brief Get data from register(s) from the device
     * @param regAddress - Address of the first register to get
     * @param regData - Buffer to store the data from the registers
     * @param len - Length of the data to get
     * @return Scheduler status
     */
    RetType getRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();
        this->i2cAddr.mem_addr = regAddress;

        RetType ret = CALL(mI2C->read(this->i2cAddr, regData, len));
        ERROR_CHECK(ret);

        RESET();
        return ret;
    }

    /**
     * @brief Parses the calibration data from the device's registers
     * @param calibrationData - Buffer containing the calibration data
     */
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


    /**
     * @brief Compensates the raw pressure data and returns the compensated pressure data
     * @param uncompData - Contains the uncompensated pressure data
     * @param calibrationData - Pointer to the calibration data structure
     * @return Data received is good or not
     */
    RetType compensateData(bmp3_uncomp_data *uncompData, bmp3_calib_data *calibrationData) {
        RetType tempRet = compensateTemperature(&this->data.temperature, uncompData, calibrationData);
        RetType pressureRet = compensatePressure(&this->data.pressure, uncompData, calibrationData);

        RetType ret;
        if (RET_SUCCESS != ret || RET_SUCCESS != pressureRet) {
            ret = RET_ERROR;
        } else {
            ret = RET_SUCCESS;
        }

        return ret;
    }


    /**
     * Compensates pressure data based on calibration data
     * @param pressure - Pointer to the variable which stores the pressure data
     * @param uncompData - Contains the uncompensated pressure data
     * @param calibrationData - Pointer to the calibration data structure
     * @return Data received is good or not
     */
    RetType compensatePressure(double *pressure, const struct bmp3_uncomp_data *uncompData,
                               const struct bmp3_calib_data *calibrationData) {
        RetType ret = RET_SUCCESS;
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
            ret = RET_ERROR;
        }

        if (comp_press > BMP3_MAX_PRES_DOUBLE) {
            comp_press = BMP3_MAX_PRES_DOUBLE;
            ret = RET_ERROR;
        }

        (*pressure) = comp_press;
        return ret;
    }

    /**
     * Compensates temperature data based on calibration data
     * @param temperature - Pointer to the variable which stores the temperature data
     * @param uncompData - Contains the uncompensated temperature data
     * @param calibrationData - Pointer to the calibration data structure
     * @return Data received is good or not
     */
    RetType compensateTemperature(double *temperature, const struct bmp3_uncomp_data *uncompData,
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
        RetType ret;
        if (calibData->quantized_calib_data.t_lin < BMP3_MIN_TEMP_DOUBLE) {
            calibData->quantized_calib_data.t_lin = BMP3_MIN_TEMP_DOUBLE;
            ret = RET_ERROR;
        }

        if (calibData->quantized_calib_data.t_lin > BMP3_MAX_TEMP_DOUBLE) {
            calibData->quantized_calib_data.t_lin = BMP3_MAX_TEMP_DOUBLE;
            ret = RET_ERROR;
        }

        (*temperature) = calibData->quantized_calib_data.t_lin;
        return RET_ERROR;
    }

    /**
     * Toggles pressure and temperature measurement
     * @param desiredSettings - Variable used to select the settings which are to be set in the sensor
     * @return Scheduler status
     */
    RetType setPowerControl(uint32_t desiredSettings) {
        RESUME();


        RetType ret = CALL(getRegister(BMP3_REG_PWR_CTRL, mBuff, 1));
        ERROR_CHECK(ret);

        if (desiredSettings & BMP3_SEL_PRESS_EN) {
            mBuff[0] = BMP3_SET_BITS_POS_0(mBuff[0], BMP3_PRESS_EN, this->settings.press_en);
        }

        if (desiredSettings & BMP3_SEL_TEMP_EN) {
            mBuff[0] = BMP3_SET_BITS(mBuff[0], BMP3_TEMP_EN, this->settings.temp_en);
        }

        ret = CALL(setRegister(BMP3_REG_PWR_CTRL, mBuff, 1));
        ERROR_CHECK(ret);

        RESET();
        return ret;
    }

    /**
     * Sets the over sampling, odr and filter settings in the sensor
     * @param desiredSettings - Variable used to select the settings which are to be set in the sensor
     * @return Scheduler status
     */
    RetType setODRFilter(uint32_t desiredSettings) {
        RESUME();

        mBuff[4] = 0; // mBuff[4] is len
        // mBuff[5] is regAddr

        RetType ret = CALL(getRegister(BMP3_REG_OSR, &mBuff[0], 4));
        ERROR_CHECK(ret);

        // OSR Data
        if (areSettingsChanged((BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS), desiredSettings)) {
            fillOSRData(desiredSettings, &mBuff[5], &mBuff[0], &mBuff[4]);
        }

        // ODR Data
        if (areSettingsChanged(BMP3_SEL_ODR, desiredSettings)) {
            fillOdrData(&mBuff[5], &mBuff[0], &mBuff[4]);
        }

        // Filter Data
        if (areSettingsChanged(BMP3_SEL_IIR_FILTER, desiredSettings)) {
            fillFilterData(&mBuff[5], &mBuff[0], &mBuff[4]);
        }

        // OSR ODR Validation
        if (settings.op_mode == BMP3_MODE_NORMAL) {
            ret = validateOsrOdr() ? RET_SUCCESS : RET_ERROR;
            ERROR_CHECK(ret);
        }

        // Burst Write
        ret = CALL(setRegister(&mBuff[5], &mBuff[0], mBuff[4]));
        ERROR_CHECK(ret);

        RESET();
        return ret;
    }

    /**
     * Fills buffer with over sampling register data
     * @param desiredSettings - Variable used to select the settings which are to be set in the sensor
     * @param addr -
     * @param regData -
     * @param len
     */
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

    /**
     * Fills buffer with output data rate register data
     * @param addr
     * @param regData
     * @param len
     */
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


    /**
     * Fills buffer with filter data
     * @param addr
     * @param regData
     * @param len
     */
    void fillFilterData(uint8_t *addr, uint8_t *regData, uint8_t *len) {

        /* Set the iir settings in the register variable */
        regData[*len] = BMP3_SET_BITS(regData[3], BMP3_IIR_FILTER, settings.odr_filter.iir_filter);

        /* 0x1F is the register address of iir filter register */
        addr[*len] = BMP3_REG_CONFIG;
        (*len)++;
    }

    /**
     * Sets the interrupt control settings of the device
     * @param desiredSettings - Variable used to select the settings which are to be set in the sensor
     * @return Scheduler status
     */
    RetType setIntCtrl(uint32_t desiredSettings) {
        RESUME();


        RetType ret = CALL(getRegister(BMP3_REG_INT_CTRL, mBuff, 1));
        ERROR_CHECK(ret);


        if (desiredSettings & BMP3_SEL_OUTPUT_MODE) {
            mBuff[0] = BMP3_SET_BITS_POS_0(mBuff[0], BMP3_INT_OUTPUT_MODE, settings.int_settings.output_mode);
        }

        if (desiredSettings & BMP3_SEL_LEVEL) {
            mBuff[0] = BMP3_SET_BITS(mBuff[0], BMP3_INT_LEVEL, settings.int_settings.level);
        }

        if (desiredSettings & BMP3_SEL_LATCH) {
            mBuff[0] = BMP3_SET_BITS(mBuff[0], BMP3_INT_LATCH, settings.int_settings.latch);
        }

        if (desiredSettings & BMP3_SEL_DRDY_EN) {
            mBuff[0] = BMP3_SET_BITS(mBuff[0], BMP3_INT_DRDY_EN, settings.int_settings.drdy_en);
        }

        ret = CALL(setRegister(BMP3_REG_INT_CTRL, mBuff, 1));

        RESET();
        return ret;
    }

    /**
     * Sets advanced settings such as i2c watchdog timeout enable and i2c watchdog timeout select
     * @param desiredSettings - Variable used to select the settings which are to be set in the sensor
     * @return Scheduler status
     */
    RetType setAdvSettings(uint32_t desiredSettings) {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_IF_CONF, mBuff, 1));
        ERROR_CHECK(ret);

        if (desiredSettings & BMP3_SEL_I2C_WDT_EN) {
            mBuff[0] = BMP3_SET_BITS(mBuff[0], BMP3_I2C_WDT_EN, settings.adv_settings.i2c_wdt_en);
        }

        if (desiredSettings & BMP3_SEL_I2C_WDT) {
            mBuff[0] = BMP3_SET_BITS(mBuff[0], BMP3_I2C_WDT_SEL, settings.adv_settings.i2c_wdt_sel);
        }

        ret = CALL(setRegister(BMP3_REG_IF_CONF, mBuff, 1));
        ERROR_CHECK(ret);


        RESET();
        return ret;
    }

    /**
     * Get the current power mode of the sensor
     * @param opMode - Variable used to store the power mode
     * @return Scheduler status
     */
    RetType getOperatingMode(uint8_t *opMode) {
        RESUME();

        RetType ret = CALL(getRegister(BMP3_REG_PWR_CTRL, opMode, 1));
        ERROR_CHECK(ret);

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

    /**
     * Check if settings are changed
     * @param subSettings - Old settings
     * @param desiredSettings - New settings
     * @return Whether settings were changed
     */
    bool areSettingsChanged(uint32_t subSettings, uint32_t desiredSettings) {
        if ((subSettings & desiredSettings)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Calculate the pressure measurement duration
     * @return
     */
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

    /**
     * Calculate the temperature measurement duration
     * @return
     */
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

    /**
     * Parses raw sensor data
     * @param reg_data
     * @param uncomp_data
     */
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

    /**
     * Parse settings data
     * @param reg_data
     */
    void parseSettings(const uint8_t *reg_data) {
        parseIntCtrlSettings(&reg_data[0]);
        parseAdvancedSettings(&reg_data[1]);
        parsePowerCtrlSettings(&reg_data[2]);
        parseODRFilterSettings(&reg_data[3]);
    }

    /**
     * Parse interrupt control settings
     * @param reg_data
     */
    void parseIntCtrlSettings(const uint8_t *reg_data) {
        settings.int_settings.output_mode = BMP3_GET_BITS_POS_0(*reg_data, BMP3_INT_OUTPUT_MODE);
        settings.int_settings.level = BMP3_GET_BITS(*reg_data, BMP3_INT_LEVEL);
        settings.int_settings.latch = BMP3_GET_BITS(*reg_data, BMP3_INT_LATCH);
        settings.int_settings.drdy_en = BMP3_GET_BITS(*reg_data, BMP3_INT_DRDY_EN);
    }

    /**
     * Parse advanced settings
     * @param reg_data
     */
    void parseAdvancedSettings(const uint8_t *reg_data) {
        settings.adv_settings.i2c_wdt_en = BMP3_GET_BITS(*reg_data, BMP3_I2C_WDT_EN);
        settings.adv_settings.i2c_wdt_sel = BMP3_GET_BITS(*reg_data, BMP3_I2C_WDT_SEL);
    }

    /**
     * Parse power control settings
     * @param reg_data
     */
    void parsePowerCtrlSettings(const uint8_t *reg_data) {
        settings.op_mode = BMP3_GET_BITS(*reg_data, BMP3_OP_MODE);
        settings.press_en = BMP3_GET_BITS_POS_0(*reg_data, BMP3_PRESS_EN);
        settings.temp_en = BMP3_GET_BITS(*reg_data, BMP3_TEMP_EN);
    }

    /**
     * Parse ODR and filter settings
     * @param reg_data
     */
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