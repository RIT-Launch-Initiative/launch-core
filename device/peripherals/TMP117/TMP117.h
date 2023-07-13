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


#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitVal) (bitVal ? bitSet(value, bit) : bitClear(value, bit))
#define TMP117_DATA_STRUCT(variable_name) TMP117::TMP117_DATA_T variable_name = {.id = 16001, .temperature = 0}


class TMP117 : public Device {
public:
    typedef struct {
        const uint16_t id;
        float temperature;
    } TMP117_DATA_T;

    typedef enum {
        TMP117_TEMP_RESULT = 0x00,
        TMP117_CONFIGURATION = 0x01,
        TMP117_T_HIGH_LIMIT = 0x02,
        TMP117_T_LOW_LIMIT = 0x03,
        TMP117_EEPROM_UL = 0x04,
        TMP117_EEPROM1 = 0x05,
        TMP117_EEPROM2 = 0x06,
        TMP117_TEMP_OFFSET = 0x07,
        TMP117_EEPROM3 = 0x08,
        TMP117_DEVICE_ID = 0x0F
    } TMP117_REGISTER_T;

    typedef enum {
        CONTINUOUS_CONVERSION = 0b00,
        ONE_SHOT = 0b11,
        SHUTDOWN = 0b01
    } TMP117_MODE_T;

    typedef enum {
        TMP117_HI_REG = 0x02,
        TMP117_LO_REG = 0x03,
    } TMP117_HILO_LIMIT_T;

    typedef enum {
        TMP117_HI_BIT = 15,
        TMP117_LO_BIT = 14,
    } TMP117_HILO_ALERT_BIT_T;

    constexpr static uint8_t TMP_117_DEVICE_ADDR =  0x48;
    constexpr static uint16_t DEVICE_ID_VALUE = 0x0117;
    constexpr static float TMP117_RESOLUTION = 0.0078125f;

    explicit TMP117(I2CDevice &i2CDevice, const uint16_t address = TMP_117_DEVICE_ADDR, const char *name = "TMP117") :
    Device(name), mI2C(&i2CDevice), i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

    RetType init() override {
        RESUME();

        RetType ret = CALL(check_id());

        RESET();
        return ret;
    }

    RetType getData(TMP117_DATA_T *data) {
        RESUME();

        RetType ret = CALL(readTempCelsius(&data->temperature));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }


    // TODO: Add parentheses to be more explicit
    RetType readTempCelsius(float *temp) {
        RESUME();

        static uint8_t data[2] = {};
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

        static uint8_t initOffset[2] = {};
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

    RetType getLimit(int16_t *limit, TMP117_HILO_LIMIT_T hilo) {
        RESUME();

        static uint8_t limit8[2] = {};
        RetType ret = CALL(readRegister(hilo, limit8, 2));
        if (ret != RET_SUCCESS) return ret;

        *limit = uint8ToInt16(limit8);

        RESET();
        return RET_SUCCESS;
    }

    RetType setLimit(float limit, TMP117_HILO_LIMIT_T hilo) {
        RESUME();

        static int16_t resolutionLimit = limit / TMP117_RESOLUTION;
        static uint8_t limit8[2] = {};
        int16ToUint8(resolutionLimit, limit8);

        RetType ret = CALL(writeRegister(hilo, limit8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType isDataReady(bool *dataReady) {
        RESUME();

        static uint8_t *response;

        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, response, 2));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        *dataReady = ((response[0] << 8) | response[1]) & 1 << 13;

        RESET();
        return RET_SUCCESS;
    }

    // TODO: See if this function can be reused more without breaking
    RetType getConfigRegister(uint16_t *configRegister) {
        RESUME();

        static uint8_t configRegister8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, configRegister8, 2));
        if (ret != RET_SUCCESS) return ret;

        *configRegister = uint8ToInt16(configRegister8); // TODO: Is Int16 conversion going to be ok?

        RESET();
        return RET_SUCCESS;
    }

    RetType getHighLowAlert(uint8_t *alert) {
        RESUME();

        static uint8_t configRegister8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, configRegister8, 2));
        if (ret != RET_SUCCESS) return ret;

        uint16_t configRegister16 = uint8ToInt16(configRegister8); // TODO: Is Int16 conversion going to be ok?

        bitWrite(*alert, 1, bitRead(configRegister16, 15));
        bitWrite(*alert, 0, bitRead(configRegister16, 14));

        RESET();
        return RET_SUCCESS;
    }

    RetType getAlert(bool *alertResult, TMP117_HILO_ALERT_BIT_T alertBit) {
        RESUME();

        static uint8_t configRegister8[2] = {};
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

        static uint8_t alertFunctionMode8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, alertFunctionMode8, 2));
        if (ret != RET_SUCCESS) return ret;

        static uint16_t alertFunctionMode16 = uint8ToInt16(alertFunctionMode8);

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
        static uint8_t configRegister8[2] = {};
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
        static uint8_t reset8[2] = {};
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, reset8, 2));
        if (ret != RET_SUCCESS) return ret;

        static uint16_t reset16 = uint8ToInt16(reset8);
        bitWrite(reset16, 1, 1);
        int16ToUint8(reset16, reset8);

        ret = CALL(writeRegister(TMP117_CONFIGURATION, reset8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType setContinuousConversionMode(TMP117_MODE_T mode) {
        RESUME();
        static uint8_t mode8[2] = {};
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

    RetType getConversionMode(TMP117_MODE_T *mode) {
        RESUME();
        static uint16_t *configRegister;
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
        static uint16_t *configRegister;
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

        static uint8_t configReg8[2] = {};
        uint16ToUint8(*configRegister, configReg8);

        ret = CALL(writeRegister(TMP117_CONFIGURATION, configReg8, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType getConversionAverageMode(uint8_t *mode) {
        RESUME();

        static uint16_t *configReg;
        RetType ret = CALL(getConfigRegister(configReg));
        if (ret != RET_SUCCESS) return ret;

        static uint8_t currMode6 = bitRead(*configReg, 6);
        static uint8_t currMode5 = bitRead(*configReg, 5);

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
        static uint16_t *configReg;
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
        static uint16_t configReg = 0;
        static uint8_t data = static_cast<uint8_t>(configReg >> 8);
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
    uint8_t tx_buff[2]{};
    uint8_t rx_buff[2]{};

    RetType check_id() {
        RESUME();

        i2cAddr.mem_addr = TMP117_DEVICE_ID;
        RetType ret = CALL(mI2C->read(i2cAddr, rx_buff, 2, 1000));
        if ((RET_SUCCESS == ret) && (DEVICE_ID_VALUE != static_cast<uint16_t>(rx_buff[0] << 8 | rx_buff[1]))) {
            ret = RET_ERROR;
        }

        RESET();
        return ret;
    }

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
