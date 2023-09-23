/*
 * INA219 Driver
 *
 * @author Brian Takamoto, Torin Samples 
*/



#ifndef INA219_H
#define INA219_H

#include <stdint.h>

#include "sched/macros.h"
#include "device/Device.h"
#include "device/I2CDevice.h"

#define INA219_BATTERY_ADDR UINT8_C(0x40)
#define INA219_3V3RAIL_ADDR UINT8_C(0x44)
#define INA219_5V0RAIL_ADDR UINT8_C(0x41)

#define INA219_DATA_STRUCT(variable_name) INA219_DATA_T variable_name = {.shuntVoltage = 0, .busVoltage = 0, .power = 0, .current = 0}

using INA219_DATA_T = struct {
    uint16_t shuntVoltage;
    uint16_t busVoltage;
    uint16_t power;
    uint16_t current;
};

class INA219 : public Device {
public:
    INA219(I2CDevice &i2cDev, const uint16_t address = INA219_BATTERY_ADDR, const char *name = "INA219") 
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
     * @brief Wrapper for filling in INA219_DATA_T struct
     * @param data - Pointer to INAXX_DATA_T struct
     * @return Scheduler Status
     */
    RetType getData(INA219_DATA_T *data) {
        RESUME();
        RetType ret = CALL(getInfo(&data->shuntVoltage, &data->busVoltage, &data->power, &data->current));

        RESET();
        return ret;
    }

    RetType getInfo(uint16_t *shuntVoltage, uint16_t *busVoltage, uint16_t *power, uint16_t *current) {
        RESUME();

        RetType ret = CALL(getRegister(INA_REG_DATA, mBuff, INA_LEN_P_T_DATA));
        ERROR_CHECK(ret);
        /** 
        struct ina219_uncomp_data uncompensatedData = {0};
        parseSensorData(mBuff, &uncompensatedData);
        ret = compensateData(&uncompensatedData, &this->device.calib_data); // Bounds checked here
        **/
        if (RET_SUCCESS == ret) {
            *huntVoltage = this->data.huntVoltage;
            *busVoltage = this->data.busVoltage;
            *power = this-> data.power;
            *current = this ->data.current;
        }

        RESET();
        return ret;
    }


    /*************************************************************************************
     * Settings
     *************************************************************************************/

private:
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;
     uint8_t mBuff[INA219_LEN_CALIB_DATA]; // INA219_LEN_CALIB_DATA is largest size

    RetType read_reg(uint8_t reg, uint8_t *buff, size_t len) {
        RESUME();

        RESET();
    }

    RetType write_reg(uint8_t reg, uint8_t *buff, size_t len) {
        uint8_t temporaryBuffer[len * 2];
        size_t temporaryLen = len;

    }

    RetType getShuntVolt(){
        RESUME();
        RESET();
    }



    RetType getBusVolt(){
        RESUME();
        RESET();
    }

    RetType getPower(){
        RESUME();
        RESET();
    }

    RetType getCurrent(){
        RESUME();
        RESET();
    }

    /**
     * @brief Check if the chip ID is correct for the INA219
     * @return Scheduler status
     */
     RetType checkChipID() {
        RESUME();

        RESET();
        return ret;
     }

     /**
     * @brief Soft reset the INA sensor
     * @return
     */
     RetType softReset() {
        RESUME();

        RESET();
        return ret;
    }

    /**
     * @brief Soft reset the INA sensor
     * @return
     */
    RetType softReset() {
        RESUME();

        RESET();
        return ret;
    }

    /**
     * @brief Get the calibration values for sensor data
     * @return Scheduler status
     */
    RetType getCalibrationData() {
        RESUME();

        RESET();
        return ret;
    }

     RetType getRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();
        RESET();
     }
};
 #endif