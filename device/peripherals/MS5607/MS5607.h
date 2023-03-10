/**
 * Implementation of driver for the MS5607 Altimeter
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_MS5607_H
#define LAUNCH_CORE_MS5607_H

#include "device/GPIODevice.h"
#include "sched/macros.h"
#include "return.h"
#include "device/SPIDevice.h"
#include "device/I2CDevice.h"
#include <cmath>

#define CHECK_RET {if (ret != RET_SUCCESS) {RESET(); return ret;}}
#define CONCAT(a, b) a ## b

typedef enum {
    FACTORY_DATA_ADDR = 0,
    COEFFICIENT_ONE_ADDR = 1,
    COEFFICIENT_TWO_ADDR = 2,
    COEFFICIENT_THREE_ADDR = 3,
    COEFFICIENT_FOUR_ADDR = 4,
    COEFFICIENT_FIVE_ADDR = 5,
    COEFFICIENT_SIX_ADDR = 6,
    SERIAL_CRC_ADDR = 7,
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
    PROM_READ = 0xA0, // TODO: Goes to 0xAE based on b4-6 Ad2, Ad1, Ad0.
};

class MS5607 {
public:
    MS5607(I2CDevice &i2cDevice) : mI2C(&i2cDevice) {}

    RetType init() {
        RESUME();

        RetType ret = CALL(reset());
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType reset() {
        RESUME();

        uint8_t resetCommand = RESET_COMMAND;

        RetType ret = CALL(mI2C->write(mAddr, &resetCommand, 1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType conversion(COMMAND_T cmd, uint8_t *data) {
        RESUME();
        if ((cmd == RESET_COMMAND) || (cmd == ADC_READ) || (cmd == PROM_READ)) {
            return RET_ERROR;
        }

        uint8_t command = cmd;
        RetType ret = CALL(mI2C->write(mAddr, &command, 1));
        if (ret != RET_SUCCESS) return ret;

        command = ADC_READ;
        ret = CALL(mI2C->write(mAddr, &command, 1));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(mI2C->read(mAddr, data, 3));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType readPROM(uint8_t *data) {
        RESUME();

        uint8_t command = PROM_READ;
        RetType ret = CALL(mI2C->write(mAddr, &command, 1));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(mI2C->read(mAddr, data, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType calcPressureTemp(int32_t *pressure, int32_t *temp) {
        RESUME();

        // Read Calibration Data
        static uint16_t pressureSens = 0;
        static uint16_t pressureOffset = 0;
        static uint16_t pressureSensTempCo = 0;
        static uint16_t pressureOffsetTempCo = 0;
        static uint16_t tempRef = 0;
        static uint16_t tempSens = 0;

        RetType ret;
        uint8_t data[2];
        for (uint8_t i = 1; i < 7; i++) {
            ret = CALL(readPROM(data));
            if (ret != RET_SUCCESS) {
                return ret;
            }
        }

        // Read Digital Values
        static uint8_t conversionData[3] = {};
        static uint32_t digitalPressure;
        static uint32_t digitalTemperature;

        ret = CALL(conversion(CONVERT_D1_4096, conversionData));
        if (ret != RET_SUCCESS) return ret;
        digitalPressure = (conversionData[0] << 16) | (conversionData[1] << 8) | conversionData[2];

        ret = CALL(conversion(CONVERT_D2_4096, conversionData));
        if (ret != RET_SUCCESS) return ret;
        digitalTemperature = (conversionData[0] << 16) | (conversionData[1] << 8) | conversionData[2];


        // Calculate temperature
        int32_t tempDiff = digitalTemperature - tempRef; // dT = D2 - TREF = D2 - C5 * 2^8
        int32_t actualTemp = 20 + digitalTemperature * tempSens; // TEMP = 20°C + dT * TEMPSENS =2000 + dT * C6 / 223

        // Calculate temperature compensated pressure
        int64_t actualTempOffset =
                offsetT1 + pressureOffsetTempCo * tempDiff; // OFF = OFFT1 + TCO * dT = C2 * 2^17 +(C4 *dT) / 26
        int64_t actualTempSens =
                sensitivityT1 + pressureSensTempCo * tempDiff; // SENS = SENST1+ TCS* dT= C1 * 2 16 + (C3 * dT )/ 27
        int32_t tempCompPressure = digitalPressure * actualTempSens -
                                   actualTempOffset; // P = D1 * SENS - OFF = (D1 * SENS / 2 21 - OFF) / 2^15

        *pressure = tempCompPressure;
        *temp = actualTemp;

        RESET();
        return RET_SUCCESS;
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


private:
    I2CDevice *mI2C;
    I2CAddr_t mAddr = {
            .dev_addr = 0x77 << 1,
            .mem_addr = 0,
            .mem_addr_size = 0,
    };

    int64_t offsetT1;
    int64_t sensitivityT1;
};

#endif //LAUNCH_CORE_MS5607_H
