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


class MS5607 {
public:
    MS5607(GPIODevice &psPin, GPIODevice &diPin, GPIODevice &doPin, GPIODevice &csPin, GPIODevice &sdaPin) :
            protocolSelectPin(psPin), dataInPin(diPin), dataOutPin(doPin), chipSelectPin(csPin),
            serialDataPin(sdaPin) {}

    RetType init(MS5607_SERIAL_PROTOCOL_T protocol) {
        RESUME();

        RetType ret = CALL(setProtocol(protocol));

        RESET();
        return ret;
    }


    RetType setProtocol(MS5607_SERIAL_PROTOCOL_T protocol) {
        RESUME();

        this->selectedProtocol = protocol;

        RESET();
        return RET_SUCCESS;
    }

    RetType reset() {

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
};

#endif //LAUNCH_CORE_MS5607_H
