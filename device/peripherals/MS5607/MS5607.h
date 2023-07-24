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
    float pressure;
    float temperature;
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
        MS5607_OSR_512 = 1,
        MS5607_OSR_1024 = 2,
        MS5607_OSR_2048 = 3,
        MS5607_OSR_4096 = 4,
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
        ERROR_CHECK(ret);

        setConv(MS5607_OSR_256);

        RESET();
        return ret;
    }

    RetType getData(MS5607_DATA_T *data) {
        RESUME();

        RetType ret = CALL(getPressureTemp(&data->pressure, &data->temperature));

        RESET();

        return RET_SUCCESS;
    }

    RetType getPressureTemp(float *pressure, float *temp) {
        RESUME();

        static uint32_t pressureVal;
        static uint32_t tempVal;

        RetType ret = CALL(readDigitalPressure(&pressureVal));
        ERROR_CHECK(ret);

        ret = CALL(readDigitalTemperature(&tempVal));
        ERROR_CHECK(ret);


        int32_t dT;

        int32_t calcTemp = calculateTemp(tempVal, &dT);
        int32_t calcPressure = calculateTempCompPressure(pressureVal, dT);
        calcTempCompensation(&calcTemp, &calcPressure);

        *pressure = calcPressure / 100.0f;
        *temp = calcTemp / 100.0f;

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

        RetType ret = CALL(writeReg(isD1 ? d1Conversion : d2Conversion, nullptr, 0, conversionDelay));

        RESET();
        return ret;
    }

    RetType readADC(uint32_t *adcValue) {
        RESUME();

        RetType ret = CALL(readReg(ADC_READ, m_buff, 3, 300));
        *adcValue = (m_buff[0] << 16) | (m_buff[1] << 8) | m_buff[2];

        RESET();
        return ret;
    }


    int32_t calculateTemp(uint32_t digitalTemp, int32_t *tempDiff) const {
        // dT = D2 - TREF = D2 - C5 * 2^8
        *tempDiff = digitalTemp - (tempRef * 256);
        // TEMP = 20°C + dT * TEMPSENS =2000 + dT * C6 / 2^23
        return 2000 + *tempDiff * tempSens / 8388608;
    }

    int32_t calculateTempCompPressure(uint32_t digitalPressure, int32_t tempDiff) const {
        // OFF = OFFT1 + TCO *dT = C2 *217 + (C4 * dT) / 2^6
        int32_t off = pressureOffset * 131072 + (pressureOffsetTempCo * tempDiff) / 64;
        // SENS = SENST1 + TCS * dT= C1 * 2^16 + (C3 * dT) / 2^7
        int32_t sens = pressureSens * 65536 + (pressureSensTempCo * tempDiff) / 128;
        // P = D1 * SENS - OFF = (D1 * SENS / 2^21 - OFF) / 2^15
        return ((digitalPressure * sens / 2097152) - off) / 32768;
    }

    void calcTempCompensation(int32_t *temperature, int32_t *tempOffset) {
        double T2 = 0;
        double OFF2 = 0;
        double SENS2 = 0;

        if (*temperature < 20) {
            T2 = pow(*tempOffset, 2) / pow(2, 31);
            OFF2 = 61 * pow(*temperature - 2000, 2) / pow(2, 4);
            SENS2 = 2 * pow(*temperature - 2000, 2);

            if (*temperature < -15) {
                OFF2 += 15 * pow(*temperature + 1500, 2);
                SENS2 += 8 * pow(*temperature + 1500, 2);
            }
        }

        *temperature -= T2;
        *tempOffset -= OFF2;
        *tempOffset -= SENS2;
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
