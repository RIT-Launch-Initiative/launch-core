/**
 * Platform independent implementation of TMP117 Sensor
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_TMP117_H
#define LAUNCH_CORE_TMP117_H

#include <stdint.h>
#include "return.h"
#include "device/I2CDevice.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "sched/macros/call.h"

#define TMP_117_DEVICE_ADDR 0x48
#define DEVICE_ID_VALUE 0x0117
#define TMP117_RESOLUTION 0.0078125f
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitVal) (bitVal ? bitSet(value, bit) : bitClear(value, bit))

typedef union {
    struct {
        uint8_t EMPTY: 1;            // Empty bit in register
        uint8_t TMP_SOFT_RESET: 1; // Software reset bit
        uint8_t DR_ALERT: 1;        // ALERT pin select bit
        uint8_t POL: 1;            // ALERT pin polarity bit
        uint8_t T_NA: 1;            // Therm/alert mode select
        uint8_t AVG: 2;            // Conversion averaging modes
        uint8_t CONV: 3;            // Conversion cycle bit
        uint8_t MOD: 2;            // Set conversion mode
        uint8_t EEPROM_BUSY: 1;    // EEPROM busy flag
        uint8_t DATA_READY: 1;        // Data ready flag
        uint8_t LOW_ALERT: 1;        // Low Alert flag
        uint8_t HIGH_ALERT: 1;        // High Alert flag
    } CONFIGURATION_FIELDS;
    uint16_t CONFIGURATION_COMBINED;
} CONFIGURATION_REG;

// Device ID Register used for checking if the device ID is the same as declared
typedef union {
    struct {
        uint16_t DID: 12; // Indicates the device ID
        uint8_t REV: 4;   // Indicates the revision number
    } DEVICE_ID_FIELDS;
    uint16_t DEVICE_ID_COMBINED;
} DEVICE_ID_REG;

enum TMP117_Register {
    TMP117_TEMP_RESULT = 0X00,
    TMP117_CONFIGURATION = 0x01,
    TMP117_T_HIGH_LIMIT = 0X02,
    TMP117_T_LOW_LIMIT = 0X03,
    TMP117_EEPROM_UL = 0X04,
    TMP117_EEPROM1 = 0X05,
    TMP117_EEPROM2 = 0X06,
    TMP117_TEMP_OFFSET = 0X07,
    TMP117_EEPROM3 = 0X08,
    TMP117_DEVICE_ID = 0X0F
};

enum TMP117_Mode {
    CONTINUOUS_CONVERSION = 0b00,
    ONE_SHOT = 0b11,
    SHUTDOWN = 0b01
};

enum TMP117_HILO_LIMIT {
    TMP117_HI_REG = 0X02,
    TMP117_LO_REG = 0X03,
};

enum TMP117_HILO_ALERT_BIT {
    TMP117_HI_BIT = 15,
    TMP117_LO_BIT = 14,
};


class TMP117 {
public:
    TMP117(I2CDevice *i2CDevice) : mI2C(i2CDevice) {}

    RetType init() {
        RESUME();

        i2cAddr = {
                .dev_addr = TMP_117_DEVICE_ADDR << 1,
                .mem_addr = TMP117_DEVICE_ID,
                .mem_addr_size = 1
        };

        uint8_t buff;
        RetType ret = CALL(mI2C->read(i2cAddr, &buff, 1));


        RESET();
        return ret;
    }



    // TODO: Add parentheses to be more explicit
    RetType readTempCelsius(float *temp) {
        RESUME();

        uint8_t data[2] = {};
        RetType ret = CALL(readRegister(TMP117_TEMP_RESULT, data, 2));
        if (ret != RET_SUCCESS) {
            return ret;
        }

        *temp = uint8ToInt16(data) * TMP117_RESOLUTION;

        RESET();
        return ret;
    }

    RetType readTempFahrenheit(float *temp) {
        RESUME();

        RetType ret = CALL(readTempCelsius(temp));
        if (ret != RET_SUCCESS) {
            return ret;
        }

        *temp = *temp * 9.0f / 5.0f + 32.0f;

        RESET();
        return ret;
    }

    RetType getTempOffset(float *offset) {
        RESUME();

        uint8_t initOffset[2] = {};
        RetType ret = CALL(readRegister(TMP117_TEMP_OFFSET, initOffset, 2));
        if (ret != RET_SUCCESS) return ret;

        int16_t initOffset16 = uint8ToInt16(initOffset);
        *offset = static_cast<float>(initOffset16) * TMP117_RESOLUTION;

        RESET();
        return RET_SUCCESS;
    }

    RetType setTempOffset(float offset) {
        RESUME();
        int16_t resolutionOffset = offset / TMP117_RESOLUTION;
        uint8_t offset8[2] = {static_cast<uint8_t>(resolutionOffset >> 8),
                              static_cast<uint8_t>(resolutionOffset & 0xFF)};

        RetType ret = CALL(writeRegister(TMP117_TEMP_OFFSET, offset8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getLimit(int16_t *limit, TMP117_HILO_LIMIT hilo) {
        RESUME();

        uint8_t limit8[2] = {};
        RetType ret = CALL(readRegister(hilo, limit8, 2));
        if (ret != RET_SUCCESS) return ret;

        *limit = uint8ToInt16(limit8);

        RESET();
        return RET_SUCCESS;
    }

    RetType setLimit(float limit, TMP117_HILO_LIMIT hilo) {
        RESUME();

        int16_t resolutionLimit = limit / TMP117_RESOLUTION;
        uint8_t limit8[2] = {};
        int16ToUint8(resolutionLimit, limit8);

        RetType ret = CALL(writeRegister(hilo, limit8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType isDataReady(bool *dataReady) {
        RESUME();

        uint8_t *response;

        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, response, 2));
        uint16_t response16 = (response[0] << 8) | response[1];

        *dataReady = *response & 1 << 13;

        RESET();
        return RET_SUCCESS;
    }

    // TODO: See if this function can be reused more without breaking
    RetType getConfigRegister(uint16_t *configRegister) {
        RESUME();

        uint8_t configRegister8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, configRegister8, 2));
        if (ret != RET_SUCCESS) return ret;

        *configRegister = uint8ToInt16(configRegister8); // TODO: Is Int16 conversion going to be ok?

        RESET();
        return RET_SUCCESS;
    }

    RetType getHighLowAlert(uint8_t *alert) {
        RESUME();

        uint8_t configRegister8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, configRegister8, 2));
        if (ret != RET_SUCCESS) return ret;

        uint16_t configRegister16 = uint8ToInt16(configRegister8); // TODO: Is Int16 conversion going to be ok?

        bitWrite(*alert, 1, bitRead(configRegister16, 15));
        bitWrite(*alert, 0, bitRead(configRegister16, 14));

        RESET();
        return RET_SUCCESS;
    }

    RetType getAlert(bool *alertResult, TMP117_HILO_ALERT_BIT alertBit) {
        RESUME();

        uint8_t configRegister8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, configRegister8, 2));
        if (ret != RET_SUCCESS) return ret;

        uint16_t configRegister16 = uint8ToInt16(configRegister8); // TODO: Is Int16 conversion going to be ok?
        uint8_t highAlert = bitRead(configRegister16, alertBit);

        *alertResult = highAlert == 1;

        RESET();
        return RET_SUCCESS;
    }

    RetType setAlertFunctionMode(uint8_t alertMode) {
        RESUME();

        uint8_t alertFunctionMode8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, alertFunctionMode8, 2));
        if (ret != RET_SUCCESS) return ret;

        uint16_t alertFunctionMode16 = uint8ToInt16(alertFunctionMode8);

        if (alertMode == 1) {
            bitWrite(alertFunctionMode16, 4, 1);
        } else {
            bitClear(alertFunctionMode16, 4);
        }

        uint16ToUint8(alertFunctionMode16, alertFunctionMode8);
        ret = CALL(writeRegister(TMP117_CONFIGURATION, alertFunctionMode8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getAlertFunctionMode(uint8_t *functionMode) {
        RESUME();
        uint8_t configRegister8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, configRegister8, 2));
        if (ret != RET_SUCCESS) return ret;

        uint16_t configRegister16 = uint8ToInt16(configRegister8);
        uint8_t currentAlertMode = bitRead(configRegister16, 4);

        *functionMode = currentAlertMode == 1 ? 1 : 0;

        RESET();
        return RET_SUCCESS;
    }

    RetType softReset() {
        RESUME();
        uint8_t reset8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, reset8, 2));
        if (ret != RET_SUCCESS) return ret;

        uint16_t reset16 = uint8ToInt16(reset8);
        bitWrite(reset16, 1, 1);
        int16ToUint8(reset16, reset8);

        ret = CALL(writeRegister(TMP117_CONFIGURATION, reset8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType setContinuousConversionMode(TMP117_Mode mode) {
        RESUME();
        uint8_t mode8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, mode8, 2));
        if (ret != RET_SUCCESS) return ret;

        uint16_t mode16 = uint8ToInt16(mode8);

        switch (mode) {
            case CONTINUOUS_CONVERSION:
                bitClear(mode16, 10);
                bitClear(mode16, 11);

                break;
            case ONE_SHOT:
                bitWrite(mode16, 10, 1);
                bitWrite(mode16, 11, 1);

                break;
            case SHUTDOWN:
                bitClear(mode16, 11);
                bitWrite(mode16, 10, 1);
                break;
        }

        int16ToUint8(mode16, mode8);

        ret = CALL(writeRegister(TMP117_CONFIGURATION, mode8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getConversionMode(TMP117_Mode *mode) {
        RESUME();
        uint16_t *configRegister;
        RetType ret = getConfigRegister(configRegister);
        if (ret != RET_SUCCESS) return ret;

        uint8_t currMode1 = bitRead(*configRegister, 11);
        uint8_t currMode2 = bitRead(*configRegister, 10);

        if ((currMode1 == 0) && (currMode2 == 1)) {
            *mode = SHUTDOWN;
        } else if ((currMode1 == 1) && (currMode2 == 1)) {
            *mode = ONE_SHOT;
        } else if ((currMode1 == 1) && (currMode2 == 0)) { // Impossible
            return RET_ERROR;
        } else { // 0b00 or by default
            *mode = CONTINUOUS_CONVERSION;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType setConversionAverageMode(uint8_t conversionMode) {
        RESUME();
        uint16_t *configRegister;
        RetType ret = CALL(getConfigRegister(configRegister));
        if (ret != RET_SUCCESS) return ret;

        switch (conversionMode) {
            case 0:
                bitClear(*configRegister, 5);
                bitClear(*configRegister, 6);
                break;
            case 1:
                bitClear(*configRegister, 6);
                bitWrite(*configRegister, 5, 1);
                break;
            case 2:
                bitWrite(*configRegister, 6, 1);
                bitClear(*configRegister, 5);
                break;
            case 3:
                bitWrite(*configRegister, 6, 1);
                bitWrite(*configRegister, 5, 1);
                break;
            default:
                return RET_ERROR;
        }

        uint8_t configReg8[2] = {};
        uint16ToUint8(*configRegister, configReg8);

        ret = CALL(writeRegister(TMP117_CONFIGURATION, configReg8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getConversionAverageMode(uint8_t *mode) {
        RESUME();

        uint16_t *configReg;
        RetType ret = CALL(getConfigRegister(configReg));
        if (ret != RET_SUCCESS) return ret;

        uint8_t currMode6 = bitRead(*configReg, 6);
        uint8_t currMode5 = bitRead(*configReg, 5);

        if ((currMode6 == 0) && (currMode5 == 1)) {// 8 avg conv
            *mode = 0b01;
        } else if ((currMode6 == 1) && (currMode5 == 0)) {// 32 avg conv
            *mode = 0b10;
        } else if ((currMode6 == 1) && (currMode5 == 1)) {// 64 avg conv
            *mode = 0b11;
        } else { // No avg conv
            *mode = 0b00;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType setConversionCycleBit(uint8_t convTime) {
        RESUME();
        uint16_t *configReg;
        RetType ret = getConfigRegister(configReg);
        if (ret != RET_SUCCESS) return ret;

        switch (convTime) {
            case 0:
                bitClear(*configReg, 9);
                bitClear(*configReg, 8);
                bitClear(*configReg, 7);

                break;
            case 1:
                bitClear(*configReg, 9);
                bitClear(*configReg, 8);
                bitWrite(*configReg, 7, 1);

                break;
            case 2:
                bitClear(*configReg, 9);
                bitWrite(*configReg, 8, 1);
                bitClear(*configReg, 7);

                break;
            case 3:
                bitClear(*configReg, 9);
                bitWrite(*configReg, 8, 1);
                bitWrite(*configReg, 7, 1);

                break;
            case 4:
                bitWrite(*configReg, 9, 1);
                bitClear(*configReg, 8);
                bitClear(*configReg, 7);

                break;
            case 5:
                bitWrite(*configReg, 9, 1);
                bitClear(*configReg, 8);
                bitWrite(*configReg, 7, 1);

                break;
            case 6:
                bitWrite(*configReg, 9, 1);
                bitWrite(*configReg, 8, 1);
                bitClear(*configReg, 7);

                break;
            case 7:
                bitWrite(*configReg, 9, 1);
                bitWrite(*configReg, 8, 1);
                bitWrite(*configReg, 7, 1);

                break;
            default:
                return RET_ERROR;
        }

        uint8_t configReg8[2] = {};
        uint16ToUint8(*configReg, configReg8);
        ret = CALL(writeRegister(TMP117_CONFIGURATION, configReg8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getConversionCycleBit(uint8_t *cycleBit) {
        RESUME();
        uint16_t configReg = 0;
        uint8_t data = static_cast<uint8_t>(configReg >> 8);
        configReg = CALL(readRegister(TMP117_CONFIGURATION, &data, sizeof(data)));

        uint8_t currentTime9 = bitRead(configReg, 9);
        uint8_t currentTime8 = bitRead(configReg, 8);
        uint8_t currentTime7 = bitRead(configReg, 7);

        if (currentTime9 == 0 && currentTime8 == 0 && currentTime7 == 0) {
            *cycleBit = 0b000;
        } else if (currentTime9 == 0 && currentTime8 == 0 && currentTime7 == 1) {
            *cycleBit = 0b001;
        } else if (currentTime9 == 0 && currentTime8 == 1 && currentTime7 == 0) {
            *cycleBit = 0b010;
        } else if (currentTime9 == 0 && currentTime8 == 1 && currentTime7 == 1) {
            *cycleBit = 0b011;
        } else if (currentTime9 == 1 && currentTime8 == 0 && currentTime7 == 0) {
            *cycleBit = 0b100;
        } else if (currentTime9 == 1 && currentTime8 == 0 && currentTime7 == 1) {
            *cycleBit = 0b101;
        } else if (currentTime9 == 1 && currentTime8 == 1 && currentTime7 == 0) {
            *cycleBit = 0b110;
        } else if (currentTime9 == 1 && currentTime8 == 1 && currentTime7 == 1) {
            *cycleBit = 0b111;
        }

        RESET();
        return RET_SUCCESS;
    }

private:
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;

    int16_t uint8ToInt16(uint8_t *data) {
        return (data[0] << 8) | data[1];
    }

    void int16ToUint8(int16_t data16, uint8_t *data8) {
        data8[0] = static_cast<uint8_t>(data16 >> 8);
        data8[1] = static_cast<uint8_t>(data16 & 0xFF);
    }

    void uint16ToUint8(uint16_t data16, uint8_t *data8) {
        data8[0] = static_cast<uint8_t>(data16 >> 8);
        data8[1] = static_cast<uint8_t>(data16 & 0xFF);
    }

    RetType readRegister(uint8_t reg, uint8_t *data, size_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C->read(i2cAddr, data, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType writeRegister(uint8_t reg, uint8_t *data, size_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C->write(i2cAddr, data, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }
};

#endif //LAUNCH_CORE_TMP117_H