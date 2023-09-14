/*
 * INA219 Driver
 *
 * @author Brian Takamoto, Torin Samples 
*/



#ifndef INA219_H
#define INA219_H
#define INA219_DATA_STRUCT(variable_name) INA219_DATA_T variable_name = {.batteryCurrentAddress = 1000000, .railCurrentAddress3V3 = 1000100, .railCurrentAddress5V0 = 1000001, .shuntVoltage = 0, .busVoltage = 0, .power = 0, .current = 0}

using INA219_DATA_T = struct {
    const uint16_t batteryCurrentAddress;
    const uint16_t railCurrentAddress3V3;
    const uint16_t railCurrentAddress5V0;
    uint16_t shuntVoltage;
    uint16_t busVoltage;
    uint16_t power;
    uint16_t current;
};

class INA219 : public Device {
    public:
    INA219(I2CDevice &i2cDev, const)
}