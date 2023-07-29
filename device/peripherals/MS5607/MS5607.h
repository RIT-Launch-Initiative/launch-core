/**
 * Implementation of driver for the MS5607 Altimeter
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_MS5607_H
#define LAUNCH_CORE_MS5607_H
#define MS5607_DATA_STRUCT(variable_name) MS5607_DATA_T variable_name = {.id = 10000, .pressure = 0, .temperature = 0}

#include "device/GPIODevice.h"
#include "sched/macros.h"
#include "return.h"
#include "device/SPIDevice.h"
#include "device/I2CDevice.h"
#include <cmath>

#define CHECK_RET {if (ret != RET_SUCCESS) {RESET(); return ret;}}
#define CONCAT(a, b) a ## b


using MS5607_DATA_T = struct {
    const uint16_t id;
    int32_t pressure;
    int32_t temperature;
};


class MS5607 : public Device {
public:
    typedef enum {
        FACTORY_DATA_ADDR = 0xA0,
        COEFFICIENT_ONE_ADDR = 0xA2,
        COEFFICIENT_TWO_ADDR = 0xA4,
        COEFFICIENT_THREE_ADDR = 0xA6,
        COEFFICIENT_FOUR_ADDR = 0xA8,
        COEFFICIENT_FIVE_ADDR = 0xAA,
        COEFFICIENT_SIX_ADDR = 0xAC,
        SERIAL_CRC_ADDR = 0xAE,
    } PROM_ADDR_T;


    enum COMMAND_T {
        RESET_COMMAND = 0x1E,

        CONVERT_D1_256 = 0x40,
        CONVERT_D1_512 = 0x42,
        CONVERT_D1_1024 = 0x44,
        CONVERT_D1_2048 = 0x46,
        CONVERT_D1_4096 = 0x48,

        CONVERT_D2_256 = 0x50,
        CONVERT_D2_512 = 0x52,
        CONVERT_D2_1024 = 0x54,
        CONVERT_D2_2048 = 0x56,
        CONVERT_D2_4096 = 0x58,

        ADC_READ = 0x00,
        PROM_READ = 0xA0,
    };

    typedef enum {
        MS5607_OSR_256 = 0,
        MS5607_OSR_512,
        MS5607_OSR_1024,
        MS5607_OSR_2048,
        MS5607_OSR_4096,
    } MS5607_OSR_T;

    static constexpr uint8_t MS5607_PRIMARY_ADDRESS = 0x76;
    static constexpr uint8_t MS5607_SECONDARY_ADDRESS = 0x77;

    MS5607(I2CDevice &i2cDevice, const uint8_t address = MS5607_PRIMARY_ADDRESS, const char *name = "MS5607") : Device(name),
    mI2C(&i2cDevice), mAddr({.dev_addr = static_cast<uint8_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

    RetType init() override {
        RESUME();

        RetType ret = CALL(reset());
        ERROR_CHECK(ret);

        SLEEP(280); // 2.8ms delay for register reload

        ret = CALL(readCalibration());
        if (RET_SUCCESS == ret) {
            setConv(MS5607_OSR_4096);
        }

        RESET();
        return ret;
    }

    RetType getData(MS5607_DATA_T *data) {
        RESUME();

        RetType ret = CALL(getPressureTemp(&data->pressure, &data->temperature));

        RESET();
        return ret;
    }

        RetType getPressureTemp(int32_t *pressure, int32_t *temp) {
        RESUME();

        static uint32_t pressureVal;
        static uint32_t tempVal;

        RetType ret = CALL(readDigitalPressure(&pressureVal));
        ERROR_CHECK(ret);

        ret = CALL(readDigitalTemperature(&tempVal));
        ERROR_CHECK(ret);

        MS5607_DATA_T calculatedData = pressureTempCalculation(pressureVal, tempVal);
        *pressure = calculatedData.pressure;
        *temp = calculatedData.temperature;

        RESET();
        return RET_SUCCESS;
    }

    RetType readDigitalPressure(uint32_t *digitalPressure) {
        RESUME();

        RetType ret = CALL(conversion(true));
        if (RET_SUCCESS == ret) {
            ret = CALL(readADC(digitalPressure));
        }

        RESET();
        return ret;
    }



    RetType readDigitalTemperature(uint32_t *digitalTemperature) {
        RESUME();

        RetType ret = CALL(conversion(false));
        if (RET_SUCCESS == ret) {
            ret = CALL(readADC(digitalTemperature));
        }

        RESET();
        return ret;
    }



    RetType reset() {
        RESUME();

        m_buff[0] = RESET_COMMAND;
        RetType ret = CALL(mI2C->transmit(mAddr, m_buff, 1, 3000));

        RESET();
        return ret;
    }


private:
    I2CDevice *mI2C;
    I2CAddr_t mAddr;

    uint8_t m_buff[10] = {0};

    uint8_t d1Conversion = CONVERT_D1_256;
    uint8_t d2Conversion = CONVERT_D2_256;
    uint16_t conversionDelay = 0;

    uint16_t pressureSens = 0;
    uint16_t pressureOffset = 0;
    uint16_t pressureSensTempCo = 0;
    uint16_t pressureOffsetTempCo = 0;
    uint16_t tempRef = 0;
    uint16_t tempSens = 0;

    RetType readReg(uint8_t command, uint8_t *buff, size_t len = 1, uint32_t timeout = 0) {
        RESUME();

        mAddr.mem_addr = command;
        RetType ret = CALL(mI2C->read(mAddr, buff, len, timeout));

        RESET();
        return ret;
    }

    RetType writeReg(uint8_t command, uint8_t *buff, size_t len = 1, uint32_t timeout = 0) {
        RESUME();

        mAddr.mem_addr = command;
        RetType ret = CALL(mI2C->write(mAddr, buff, len, timeout));

        RESET();
        return ret;
    }

    RetType readCalibration() {
        RESUME();

        RetType ret = CALL(readReg(COEFFICIENT_ONE_ADDR, m_buff, 2, 300));
        if (RET_SUCCESS == ret) pressureSens = (m_buff[0] << 8) | m_buff[1];
        else ERROR_CHECK(RET_ERROR);

        ret = CALL(readReg(COEFFICIENT_TWO_ADDR, m_buff, 2, 300));
        if (RET_SUCCESS == ret) pressureOffset = (m_buff[0] << 8) | m_buff[1];
        else ERROR_CHECK(RET_ERROR);

        ret = CALL(readReg(COEFFICIENT_THREE_ADDR, m_buff, 2, 300));
        if (RET_SUCCESS == ret) pressureSensTempCo = (m_buff[0] << 8) | m_buff[1];
        else ERROR_CHECK(RET_ERROR);

        ret = CALL(readReg(COEFFICIENT_FOUR_ADDR, m_buff, 2, 300));
        if (RET_SUCCESS == ret) pressureOffsetTempCo = (m_buff[0] << 8) | m_buff[1];
        else ERROR_CHECK(RET_ERROR);

        ret = CALL(readReg(COEFFICIENT_FIVE_ADDR, m_buff, 2, 300));
        if (RET_SUCCESS == ret) tempRef = (m_buff[0] << 8) | m_buff[1];
        else ERROR_CHECK(RET_ERROR);

        ret = CALL(readReg(COEFFICIENT_SIX_ADDR, m_buff, 2, 300));
        if (RET_SUCCESS == ret) tempSens = (m_buff[0] << 8) | m_buff[1];
        else ERROR_CHECK(RET_ERROR);

        RESET();
        return ret;
    }

    RetType conversion(const bool isD1) {
        RESUME();

        if (isD1) {
            m_buff[0] = d1Conversion;
        } else {
            m_buff[0] = d2Conversion;
        }

        RetType ret = CALL(mI2C->transmit(mAddr, m_buff, 1, 10));
        SLEEP(conversionDelay);

        RESET();
        return RET_SUCCESS;
    }

    RetType readADC(uint32_t *adcValue) {
        RESUME();

        RetType ret = CALL(readReg(ADC_READ, m_buff, 3, 300));
        if (RET_SUCCESS == ret) {
            *adcValue = (m_buff[0] << 16) | (m_buff[1] << 8) | m_buff[2];
        }

        RESET();
        return ret;
    }


    MS5607_DATA_T pressureTempCalculation(uint32_t D1, uint32_t D2) {
        // Calibration Data
        uint16_t C1 = pressureSens;
        uint16_t C2 = pressureOffset;
        uint16_t C3 = pressureSensTempCo;
        uint16_t C4 = pressureOffsetTempCo;
        uint16_t C5 = tempRef;
        uint16_t C6 = tempSens;

        // Data already read

        // Calculate Temperature
        int32_t dT = static_cast<int32_t>(D2) - (C5 << 8);
        int32_t TEMP = 2000 + dT * C6 / (1 << 23);

        // Calculate Pressure
        int64_t OFF = C2 * (1 << 17) + (C4 * dT) / (1 << 6);
        int64_t SENS = C1 * (1 << 16) + (C3 * dT) / (1 << 7);
        int32_t P = (static_cast<int32_t>(D1) * SENS / (1 << 21) - OFF) / (1 << 15);

        return {.id = 0, .pressure = P, .temperature = TEMP};
    }

    void setConv(MS5607_OSR_T osr) {
        switch (osr) {
            case MS5607_OSR_256:
                d1Conversion = CONVERT_D1_256;
                d2Conversion = CONVERT_D2_256;
                conversionDelay = 1;
                break;
            case MS5607_OSR_512:
                d1Conversion = CONVERT_D1_512;
                d2Conversion = CONVERT_D2_512;
                conversionDelay = 3;
                break;
            case MS5607_OSR_1024:
                d1Conversion = CONVERT_D1_1024;
                d2Conversion = CONVERT_D2_1024;
                conversionDelay = 4;
                break;
            case MS5607_OSR_2048:
                d1Conversion = CONVERT_D1_2048;
                d2Conversion = CONVERT_D2_2048;
                conversionDelay = 6;
                break;
            case MS5607_OSR_4096:
                d1Conversion = CONVERT_D1_4096;
                d2Conversion = CONVERT_D2_4096;
                conversionDelay = 10;
                break;
            default:
                d1Conversion = CONVERT_D1_256;
                d2Conversion = CONVERT_D2_256;
                conversionDelay = 1;
                break;
        }
    }
};

#endif //LAUNCH_CORE_MS5607_H
