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
        uint16_t raw_shunt_voltage = (mBuff[1] << 8) | (mBuff[2]);
        *shunt_vol = abs(~raw_shunt_voltage) +1;
        
        RESET();
    }



    RetType getBusVolt(int16_t *bus_vol){
        RESUME();

        RetType ret = CALL(read_reg(BUS_VOLT_REG,mBuff,2));
        int16_t raw_bus_vol = (mBuff[3] << 8) | (mBuff[4]);

        *bus_vol = raw_bus_vol;

        RESET();
    }

    RetType getPower(uint16_t *power){
        RESUME();

        RetType ret = CALL(read_reg(POWER_REG,mBuff,2));
        int16_t raw_power = (mBuff[5] << 8) | (mBuff[6]);

        *power = raw_power;

        RESET();
    }

    RetType getCurrent(uint16_t *current){
        RESUME();

        RetType ret = CALL(read_reg(CURRENT_REG,mBuff,2));
        int16_t raw_power = (mBuff[7] << 8) | (mBuff[8]);
        int16_t mask = 0x8000; // 1000000000000000 in binary

        if ( raw_power & mask ){
            *current = ~raw_power;
        } else{
            *current = raw_power;
        }
        RESET();
    }

    /**
     * @brief Check if the chip ID is correct for the INA219
     * @return Scheduler status
     */
     RetType checkChipID() {
        RESUME();
        
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

    /**
     * @brief Get the calibration values for sensor data
     * @return Scheduler status
     */
    RetType getCalibrationData() {
        RESUME();

        RESET();
    }

     RetType getRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();
        RESET();
     }
};
 #endif