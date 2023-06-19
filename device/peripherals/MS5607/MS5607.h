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

#include "stm32f4xx_hal_i2c.h"

extern I2C_HandleTypeDef hi2c3;
extern int swprintf(const char* fmt, ...);

#define CHECK_RET {if (ret != RET_SUCCESS) {RESET(); return ret;}}
#define CONCAT(a, b) a ## b


using MS5607_DATA_T = struct {
    const uint16_t id;
    float pressure;
    float temperature;
};

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

class MS5607 {
public:
    MS5607(I2CDevice &i2cDevice) : mI2C(&i2cDevice) {}

    RetType init(uint8_t address = 0x76) {
        RESUME();

        mAddr.dev_addr = address << 1;

        RetType ret = CALL(reset());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        SLEEP(5000); // 2.8ms delay for register reload
        ret = CALL(readCalibration());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        setConv(MS5607_OSR_256);

        RESET();
        return ret;
    }

    float getAltitude(float pressure, float temp) {
        return 153.84615 * (pow(pressure / 1013.25f, 0.1903f) - 1) * (temp + 273.15);
    }

    RetType getPressureTemp(float *pressure, float *temp) {
        RESUME();

        static uint32_t pressureVal;
        static uint32_t tempVal;

        RetType ret = CALL(conversion(true));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(startMeasAndGetVal(&pressureVal));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(conversion(false));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(startMeasAndGetVal(&tempVal));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }


//        ret = CALL(startMeasurement());
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//
//        ret = CALL(getDigitalVal(&pressureVal));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//
//        ret = CALL(conversion(false));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//
//        ret = CALL(startMeasurement());
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//
//        ret = CALL(getDigitalVal(&tempVal));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }

        int32_t dT;

        int32_t calcTemp = calculateTemp(tempVal, &dT);
        int32_t calcPressure = calculateTempCompPressure(pressureVal, dT);
        calcTempCompensation(&calcTemp, &calcPressure);

        *pressure = calcPressure / 100.0f;
        *temp = calcTemp / 100.0f;

        RESET();
        return RET_SUCCESS;
    }

    RetType reset() {
        RESUME();

        static uint8_t rst_cmd = RESET_COMMAND;

        RetType ret = CALL(mI2C->transmit(mAddr, &rst_cmd, 1, 3000));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return ret;
    }


private:
    I2CDevice *mI2C;
    I2CAddr_t mAddr = {
            .dev_addr = 0x76 << 1,
            .mem_addr = 0,
            .mem_addr_size = 1,
    };

    uint8_t d1Conversion = 0x40;
    uint8_t d2Conversion = 0x50;
    uint16_t conversionDelay = 0;

    uint16_t pressureSens = 0;
    uint16_t pressureOffset = 0;
    uint16_t pressureSensTempCo = 0;
    uint16_t pressureOffsetTempCo = 0;
    uint16_t tempRef = 0;
    uint16_t tempSens = 0;

    uint16_t coefficients[6] = {0};

    RetType readCalibration() {
        RESUME();

        read16Bit(COEFFICIENT_ONE_ADDR, &pressureSens);
        read16Bit(COEFFICIENT_TWO_ADDR, &pressureOffset);
        read16Bit(COEFFICIENT_THREE_ADDR, &pressureSensTempCo);
        read16Bit(COEFFICIENT_FOUR_ADDR, &pressureOffsetTempCo);
        read16Bit(COEFFICIENT_FIVE_ADDR, &tempRef);
        read16Bit(COEFFICIENT_SIX_ADDR, &tempSens);

        swprintf("pressureSens: %d\n", pressureSens);
        swprintf("pressureOffset: %d\n", pressureOffset);
        swprintf("pressureSensTempCo: %d\n", pressureSensTempCo);
        swprintf("pressureOffsetTempCo: %d\n", pressureOffsetTempCo);
        swprintf("tempRef: %d\n", tempRef);
        swprintf("tempSens: %d\n", tempSens);





//        static uint8_t buff[2];
//        buff[0] = PROM_READ + COEFFICIENT_ONE_ADDR;
//        RetType ret = CALL(mI2C->transmitReceive(mAddr, buff, 1, 2, 1000));
//        pressureSens = buff[0] << 8 | buff[1];
//
//        buff[0] = PROM_READ + COEFFICIENT_TWO_ADDR;
//        ret = CALL(mI2C->transmitReceive(mAddr, buff, 1, 2, 1000));
//        pressureOffset = buff[0] << 8 | buff[1];
//
//        buff[0] = PROM_READ + COEFFICIENT_THREE_ADDR;
//        ret = CALL(mI2C->transmitReceive(mAddr, buff, 1, 2, 1000));
//        pressureSensTempCo = buff[0] << 8 | buff[1];
//
//        buff[0] = PROM_READ + COEFFICIENT_FOUR_ADDR;
//        ret = CALL(mI2C->transmitReceive(mAddr, buff, 1, 2, 1000));
//        pressureOffsetTempCo = buff[0] << 8 | buff[1];
//
//        buff[0] = PROM_READ + COEFFICIENT_FIVE_ADDR;
//        ret = CALL(mI2C->transmitReceive(mAddr, buff, 1, 2, 1000));
//        tempRef = buff[0] << 8 | buff[1];
//
//        buff[0] = PROM_READ + COEFFICIENT_SIX_ADDR;
//        ret = CALL(mI2C->transmitReceive(mAddr, buff, 1, 2, 1000));
//        tempSens = buff[0] << 8 | buff[1];

//        static uint8_t buff[16];
//        buff[0] = PROM_READ;
//        coefficients[0] = PROM_READ + 2;
//        RetType ret = CALL(mI2C->transmitReceive(mAddr, buff, 1, 16, 1000));
//        if (RET_SUCCESS != ret) {
//            RESET();
//            return ret;
//        }
//
//        pressureSens = buff[2] << 8 | buff[3];
//        pressureOffset = buff[4] << 8 | buff[5];
//        pressureSensTempCo = buff[6] << 8 | buff[7];
//        pressureOffsetTempCo = buff[8] << 8 | buff[9];
//        tempRef = buff[10] << 8 | buff[11];
//        tempSens = buff[12] << 8 | buff[13];
//
//        pressureSens = coefficients[1];
//        pressureOffset = coefficients[2];
//        pressureSensTempCo = coefficients[3];
//        pressureOffsetTempCo = coefficients[4];
//        tempRef = coefficients[5];
//        tempSens = coefficients[6];

//        static uint8_t data[2];
//        RetType ret = CALL(readPROM(data, COEFFICIENT_ONE_ADDR));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//        pressureSens = (data[0] << 8) | data[1];
//
//        ret = CALL(readPROM(data, COEFFICIENT_TWO_ADDR));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//        pressureOffset = (data[0] << 8) | data[1];
//
//        ret = CALL(readPROM(data, COEFFICIENT_THREE_ADDR));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//        pressureSensTempCo = (data[0] << 8) | data[1];
//
//        ret = CALL(readPROM(data, COEFFICIENT_FOUR_ADDR));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//        pressureOffsetTempCo = (data[0] << 8) | data[1];
//
//        ret = CALL(readPROM(data, COEFFICIENT_FIVE_ADDR));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//        tempRef = (data[0] << 8) | data[1];
//
//        ret = CALL(readPROM(data, COEFFICIENT_SIX_ADDR));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//        tempSens = (data[0] << 8) | data[1];

        RESET();
        return RET_SUCCESS;
    }

    void read16Bit(uint8_t command, uint16_t *value) {
        uint8_t data[2];

//        HAL_I2C_Master_Transmit(&hi2c3, mAddr.dev_addr, &command, 1, 1000);
//        HAL_I2C_Master_Receive(&hi2c3, mAddr.dev_addr, data, 2, 1000);

        HAL_I2C_Mem_Read(&hi2c3, mAddr.dev_addr, command, 1, data, 2, 1000);
        *value = data[0] << 8 | data[1];


//        *value = (((unsigned int) data[0] * (1 << 8)) | (unsigned int) data[1]);
    }



    RetType readPROM(uint8_t *data, uint8_t offset) {
        RESUME();

        data[0] = PROM_READ + offset;
        RetType ret = CALL(mI2C->transmitReceive(mAddr, data, 1, 2, 300));

//        RetType ret = CALL(mI2C->transmit(mAddr, data, 1, 1000));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }
//
//        ret = CALL(mI2C->receive(mAddr, data, 2, 1000));
//        if (ret != RET_SUCCESS) {
//            RESET();
//            return ret;
//        }

        RESET();
        return ret;
    }

    RetType readPROM() {
        RESUME();

        static uint8_t buff[8];
        static uint8_t data[2];
        static int i = 0;

        RetType ret = CALL(reset());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        SLEEP(300);

        for (; i < 8; i++) {
            data[0] = PROM_READ + (i * 2);
            ret = CALL(mI2C->transmitReceive(mAddr, data, 1, 2, 50));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            buff[i] = (data[0] << 8) + data[1];
        }

        i = 0;

        pressureSens = (buff[0] << 8) | buff[1];
        pressureOffset = (buff[2] << 8) | buff[3];
        pressureSensTempCo = (buff[4] << 8) | buff[5];
        pressureOffsetTempCo = (buff[6] << 8) | buff[7];

        RESET();
        return ret;
    }

    RetType conversion(const bool isD1) {
        RESUME();

        RetType ret = CALL(mI2C->transmit(mAddr, isD1 ? &d1Conversion : &d2Conversion, conversionDelay + 5));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType startMeasAndGetVal(uint32_t *val) {
        RESUME();

        static uint8_t data[3];
        data[0] = ADC_READ;
        RetType ret = CALL(mI2C->transmitReceive(mAddr, data, 1, 3, 3000));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        *val = (data[0] << 16) | (data[1] << 8) | data[2];

        RESET();
        return ret;
    }

    RetType startMeasurement() {
        RESUME();

        RetType ret = CALL(mI2C->transmit(mAddr, reinterpret_cast<uint8_t *>(ADC_READ), 1, 4000));

        RESET();
        return ret;
    }

    RetType getDigitalVal(uint32_t *val) {
        RESUME();

        RetType ret = CALL(mI2C->receive(mAddr, reinterpret_cast<uint8_t *>(val), 3, 1000));
        RESET();
        return ret;
    }

    RetType readDigitalVals(uint32_t *pressure, uint32_t *temp) {
        RESUME();

        static uint8_t data[3];

        RetType ret = CALL(conversion(true));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        data[0] = ADC_READ;
        ret = CALL(mI2C->transmitReceive(mAddr, data, 1, 3, 100));
        if (RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        *pressure = (data[0] << 16) | (data[1] << 8) | data[2];

        ret = CALL(conversion(false));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        data[0] = ADC_READ;
        ret = CALL(mI2C->transmitReceive(mAddr, data, 1, 3, 50));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
        *temp = (data[0] << 16) | (data[1] << 8) | data[2];

        RESET();
        return RET_SUCCESS;
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

    RetType adcRead(uint32_t *pressure_adc, uint32_t *temperature_adc) {
        RESUME();

        static uint8_t data[3];

        data[0] = ADC_READ + d1Conversion;
        RetType ret = CALL(mI2C->transmitReceive(mAddr, data, 1, 3, 50));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        SLEEP(conversionDelay);

        *pressure_adc = (data[0] << 16) | (data[1] << 8) | data[2];


        data[0] = ADC_READ + d2Conversion;
        ret = CALL(mI2C->transmitReceive(mAddr, data, 1, 3, 50));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        SLEEP(conversionDelay);

        *temperature_adc = (data[0] << 16) | (data[1] << 8) | data[2];

        RESET();
        return ret;
    }

};

#endif //LAUNCH_CORE_MS5607_H
