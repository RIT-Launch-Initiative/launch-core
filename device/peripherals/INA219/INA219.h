/*
 * INA219 Driver
 *
 * @author Brian Takamoto, Torin Samples 
*/



#ifndef INA219_H
#define INA219_H

#include <stdint.h>

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

        RESET();
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

     RetType checkChipID() {
        RESUME();

        ADD_ADRESS = 9999; //add chip INA address

        this->i2cAddr.mem_addr = ADD_ADRESS;
        RetType ret = CALL(getRegister(ADD_ADRESS, mBuff, 1));

        RESET();
     }

     RetType getRegister(uint8_t regAddress, uint8_t *regData, uint32_t len) {
        RESUME();
        RESET();
     }
};