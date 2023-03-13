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

        static uint8_t data[2];
        RetType ret = CALL(reset());
        if (ret != RET_SUCCESS) return ret;

        SLEEP(280); // 2.8ms delay for register reload

        ret = CALL(readPROM(data, 0));
        if (ret != RET_SUCCESS) return ret;
        pressureSens = (data[0] << 8) | data[1];

        ret = CALL(readPROM(data, 2));
        if (ret != RET_SUCCESS) return ret;
        pressureOffset = (data[0] << 8) | data[1];

        ret = CALL(readPROM(data, 4));
        if (ret != RET_SUCCESS) return ret;
        tempSens = (data[0] << 8) | data[1];

        ret = CALL(readPROM(data, 6));
        if (ret != RET_SUCCESS) return ret;
        pressureSensTempCo = (data[0] << 8) | data[1];

        ret = CALL(readPROM(data, 8));
        if (ret != RET_SUCCESS) return ret;
        pressureOffsetTempCo = (data[0] << 8) | data[1];

        ret = CALL(readPROM(data, 10));
        if (ret != RET_SUCCESS) return ret;
        tempRef = (data[0] << 8) | data[1];

        ret = CALL(readPROM(data, 12));
        if (ret != RET_SUCCESS) return ret;
        tempSens = (data[0] << 8) | data[1];

        RESET();
        return ret;
    }

    RetType getPressureTemp(int32_t *pressure, int32_t *temp) {
        RESUME();

        static uint32_t pressureVal;
        static uint32_t tempVal;

        RetType ret = CALL(readDigitalVals(&pressureVal, &tempVal));
        if (ret != RET_SUCCESS) return ret;

        *temp = calculateTemp(tempVal);
        *pressure = calculateTempCompPressure(pressureVal, *temp);

        calcTempCompensation(temp, pressure);

        RESET();
        return RET_SUCCESS;
    }

    RetType reset() {
        RESUME();

        RetType ret = CALL(mI2C->write(mAddr, reinterpret_cast<uint8_t*>(RESET_COMMAND), 1, 10));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType conversion(COMMAND_T cmd, uint8_t *data) {
        RESUME();
        if ((cmd == RESET_COMMAND) || (cmd == ADC_READ) || (cmd == PROM_READ)) {
            return RET_ERROR;
        }

        static uint8_t command = cmd;
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

    RetType readPROM(uint8_t *data, uint8_t offset) {
        RESUME();

        RetType ret = CALL(mI2C->write(mAddr, reinterpret_cast<uint8_t*>(PROM_READ + offset), 2));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(mI2C->read(mAddr, data, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType readDigitalVals(uint32_t *pressure, uint32_t *temp) {
        RESUME();

        static uint8_t data[3];

        RetType ret = CALL(conversion(CONVERT_D1_4096, data));
        if (ret != RET_SUCCESS) return ret;
        *pressure = (data[0] << 16) | (data[1] << 8) | data[2];

        ret = CALL(conversion(CONVERT_D2_4096, data));
        if (ret != RET_SUCCESS) return ret;
        *temp = (data[0] << 16) | (data[1] << 8) | data[2];

        RESET();
        return RET_SUCCESS;
    }

    int32_t calculateTemp(uint32_t digitalTemp) const {
        // dT = D2 - TREF = D2 - C5 * 2^8
        int32_t tempDiff = digitalTemp - tempRef;
        // TEMP = 20°C + dT * TEMPSENS =2000 + dT * C6 / 223
        return 2000 + tempDiff * tempSens;

    }

    int32_t calculateTempCompPressure(uint32_t digitalPressure, int32_t tempDiff) const {
        // OFF = OFFT1 + TCO * dT = C2 * 2^17 +(C4 *dT) / 26
        int64_t actualTempOffset = offsetT1 + pressureOffsetTempCo * tempDiff;
        // SENS = SENST1+ TCS* dT= C1 * 2 16 + (C3 * dT )/ 27
        int64_t actualTempSens = sensitivityT1 + pressureSensTempCo * tempDiff;
        // P = D1 * SENS - OFF = (D1 * SENS / 2 21 - OFF) / 2^15
        return digitalPressure * actualTempSens - actualTempOffset;
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
            .mem_addr_size = 1,
    };

    uint16_t pressureSens = 0;
    uint16_t pressureOffset = 0;
    uint16_t pressureSensTempCo = 0;
    uint16_t pressureOffsetTempCo = 0;
    uint16_t tempRef = 0;
    uint16_t tempSens = 0;

    int64_t offsetT1;
    int64_t sensitivityT1;
};

#endif //LAUNCH_CORE_MS5607_H
