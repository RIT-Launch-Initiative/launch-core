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

#define CHECK_RET {if (ret != RET_SUCCESS) {RESET(); return ret;}}

typedef enum {
    I2C_PROTOCOL = 1,
    SPI_PROTOCOL = 0
} MS5607_SERIAL_PROTOCOL_T;

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

typedef enum {
    SPI_RESET_COMMAND = 0x1E,
    SPI_CONVERT_D1_256 = 0x40,
    SPI_CONVERT_D1_512 = 0x42,
    SPI_CONVERT_D1_1024 = 0x44,
    SPI_CONVERT_D1_2048 = 0x46,
    SPI_CONVERT_D1_4096 = 0x48,

    SPI_CONVERT_D2_256 = 0x50,
    SPI_CONVERT_D2_512 = 0x52,
    SPI_CONVERT_D2_1024 = 0x54,
    SPI_CONVERT_D2_2048 = 0x56,
    SPI_CONVERT_D2_4096 = 0x58,

    SPI_ADC_READ = 0x00,
    SPI_PROM_READ = 0xA0, // TODO: Goes to 0xAE based on b4-6 Ad2, Ad1, Ad0.
} SPI_COMMAND_T;


class MS5607 {
public:
    MS5607(GPIODevice &psPin, GPIODevice &diPin, GPIODevice &doPin, GPIODevice &csPin, GPIODevice &sdaPin) :
            protocolSelectPin(psPin), dataInPin(diPin), dataOutPin(doPin), chipSelectPin(csPin),
            serialDataPin(sdaPin) {}

    RetType init(MS5607_SERIAL_PROTOCOL_T protocol) {
        RESUME();

        RetType ret = CALL(setProtocol(protocol));

        // TODO: Set T1 and S1

        RESET();
        return ret;
    }


    RetType setProtocol(MS5607_SERIAL_PROTOCOL_T protocol) {
        RESUME();

        this->selectedProtocol = protocol;

        RESET();
        return RET_SUCCESS;
    }




    RetType d1Conversion() {

    }

    RetType d2Conversion() {

    }

    RetType readADC() {

    }

    RetType calcPressureTemp(int32_t *pressure, int32_t *temp) {
        RESUME();

        // Read Calibration Data
        uint16_t pressureSens = 0;
        uint16_t pressureOffset = 0;
        uint16_t pressureSensTempCo = 0;
        uint16_t pressureOffsetTempCo = 0;
        uint16_t tempRef = 0;
        uint16_t tempSens = 0;

        RetType ret;
        for (uint8_t i = 1; i < 7; i++) {
            ret = CALL(readProm(static_cast<PROM_ADDR_T>(i)));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
        }

        // Read Digital Values
        uint32_t digitalPressure = d1Conversion();
        uint32_t digitalTemperature = d2Conversion();

        // Calculate temperature
        int32_t tempDiff = digitalTemperature - tempRef; // dT = D2 - TREF = D2 - C5 * 2^8
        int32_t actualTemp = 20 + digitalTemperature * tempSens; // TEMP = 20°C + dT * TEMPSENS =2000 + dT * C6 / 223

        // Calculate temperature compensated pressure
        int64_t actualTempOffset = offsetT1 + pressureOffsetTempCo * tempDiff; // OFF = OFFT1 + TCO * dT = C2 * 2^17 +(C4 *dT) / 26
        int64_t actualTempSens = sensitivityT1 + pressureSensTempCo * tempDiff; // SENS = SENST1+ TCS* dT= C1 * 2 16 + (C3 * dT )/ 27
        int32_t tempCompPressure = digitalPressure * actualTempSens - actualTempOffset; // P = D1 * SENS - OFF = (D1 * SENS / 2 21 - OFF) / 2^15

        *pressure = tempCompPressure;
        *temp = actualTemp;

        RESET();
        return RET_SUCCESS;
    }

    RetType calcTempCompensation(int32_t temperature) {
        return RET_SUCCESS;
    }


private:
    MS5607_SERIAL_PROTOCOL_T selectedProtocol;
    GPIODevice &protocolSelectPin;

    GPIODevice &dataInPin;
    GPIODevice &dataOutPin;
    GPIODevice &chipSelectPin;

    GPIODevice &serialDataPin;

    int64_t offsetT1;
    int64_t sensitivityT1;

    RetType readProm(PROM_ADDR_T address) {

    }

    RetType reset() {
        RESUME();
        if (selectedProtocol == SPI_PROTOCOL) {
            RetType ret = CALL(chipSelectPin.set(0));
            CHECK_RET

            ret = CALL(dataOutPin.set(SPI_RESET_COMMAND));
            CHECK_RET

            ret = CALL(chipSelectPin.set(1));

            RESET();
            return ret;
        }


    }
};

#endif //LAUNCH_CORE_MS5607_H

