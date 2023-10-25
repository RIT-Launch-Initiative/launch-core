/*
 * INA219 Driver
 *
 * @author Brian Takamoto, Torin Samples 
*/



#ifndef INA219_H
#define INA219_H

#include <stdint.h>
#include <iostream>

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
    typedef enum {
        CONFIG_REG = 0x00,
        SHUNT_VOLT_REG = 0x01,
        BUS_VOLT_REG = 0x02,
        POWER_REG = 0x03,
        CURRENT_REG = 0x04,
        CALIB_REG = 0x05
    } INA219_REGISTER;

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

        ret = CALL(reset());
        ERROR_CHECK(ret);
        
        ret = CALL(getCalibrationData());
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


        RetType ret = CALL(getData(&data->shuntVoltage, &data->busVoltage, &data->power, &data->current));

        RESET();
        return ret;
    }

    RetType getData(uint16_t *shuntVoltage, uint16_t *busVoltage, uint16_t *power, uint16_t *current) {
        RESUME();

        RetType ret = CALL(read_reg(SHUNT_VOLT_REG, mBuff, 8));
        *shuntVoltage = (mBuff[2] << 8) | (mBuff[2]);
        *busVoltage = (mBuff[3] << 8) | (mBuff[4]);
        *power = (mBuff[5] << 8) | (mBuff[6]); 
        *current = (mBuff[7] << 8) | (mBuff[8]);
        RESET();
        return ret;
    }


    /*************************************************************************************
     * Settings
     *************************************************************************************/

private:
    I2CDevice &mI2C;
    I2CAddr_t i2cAddr;
     uint8_t mBuff[10]; // INA219_LEN_CALIB_DATA is largest size

    RetType read_reg(uint8_t reg, uint8_t *buff, size_t len) {
        RESUME();
        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C.read(i2cAddr, buff, len));

        RESET();
        return ret;
    }

    RetType write_reg(uint8_t reg, uint8_t *buff, size_t len) {
        RESUME();
        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C.write(i2cAddr, buff, len));

        RESET();
        return ret;

    }

    RetType getShuntVolt(int16_t *shunt_vol){
        RESUME();
        
        RetType ret = CALL(read_reg(SHUNT_VOLT_REG,mBuff,2));
        uint16_t raw_shunt_voltage = (mBuff[0] << 8) | (mBuff[1]);

        if(raw_shunt_voltage & (0b1 << 15)){
            *shunt_vol = abs(~raw_shunt_voltage) +1;
        } else {
            *shunt_vol = raw_shunt_voltage;
        }
        
        RESET();
        return RET_SUCCESS;
    }

    RetType getBusVolt(int16_t *bus_vol){
        RESUME();

        RetType ret = CALL(read_reg(BUS_VOLT_REG,mBuff,2));
        if(mBuff[1] & 0b1){
            RESET();
            return RET_ERROR;
        }
        *bus_vol = ((mBuff[0] << 8) | (mBuff[1]) >> 3);
        RESET();
        return RET_SUCCESS;
    }

    RetType getPower(uint16_t *power){
        RESUME();
        
        RetType ret = read_reg(CURRENT_REG, mBuff, 2);
        
        if (ret != SUCCESS) {
            RESET();
            return ret;
        }
        uint16_t raw_current = (mBuff[0] << 8) | mBuff[1]; // get the value in the current reg

        ret = read_reg(BUS_VOLT_REG, mBuff, 2);
        if (ret != SUCCESS) {
            RESET();
            return ret;
        }
        uint16_t raw_busVoltage = (mBuff[0] << 8) | mBuff[1]; // get the value in the busVolt reg

        
        *power = (raw_current * raw_busVoltage)/5000; 

        RESET();
        return RET_SUCCESS;
    }

    RetType getCurrent(uint16_t *current, uint16_t *power){
        RESUME();

        RetType ret = CALL(getPower(&power));

        *current = (shuntVoltage * calibration)/4096;
        RESET();
    }

    /**
     * @brief Get the calibration values for sensor data
     * @return Scheduler status
     */
    RetType getCalibrationData(uint16_t *calibraion) {
        RESUME();

        RetType ret = CALL(read_reg(SHUNT_VOLT_REG, mBuff, 10));
        uint16_t shuntVoltage = (mBuff[2] << 8) | (mBuff[2]);
        uint16_t busVoltage = (mBuff[3] << 8) | (mBuff[4]);
        uint16_t power = (mBuff[5] << 8) | (mBuff[6]); 
        uint16_t current = (mBuff[7] << 8) | (mBuff[8]);
        uint16_t calibration = (mBuff[9] << 8) | (mBuff[10]);

        float max_expected_curr = 1.0;
        float Current_LSB = max_expected_curr/(2^15);
        float Power_LSB = 20(Current_LSB);

        *calibraion = (uint16_t)(0.04096/Current_LSB*); 

        RESET();
    }

     /**
     * @brief Soft reset the INA sensor
     * @return
     */
     RetType reset() {
        RESUME();

        mBuff[0] = 0x39;
        mBuff[1] = 0x9F;
        RetType ret = CALL(write_reg(CONFIG_REG, mBuff, 2));


        RESET();
        return ret;
    }
};
 #endif