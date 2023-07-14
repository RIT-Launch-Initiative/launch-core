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
#include "utils/conversion.h"


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

    constexpr static uint8_t TMP_117_DEVICE_ADDR = 0x48;
    constexpr static uint16_t DEVICE_ID_VALUE = 0x0117;
    constexpr static float TMP117_RESOLUTION = 0.0078125f;

    explicit TMP117(I2CDevice &i2CDevice, const uint16_t address = TMP_117_DEVICE_ADDR, const char *name = "TMP117") :
            Device(name), mI2C(&i2CDevice),
            i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 1}) {}

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


    RetType readTempCelsius(float *temp) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_TEMP_RESULT, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *temp = (static_cast<int16_t>(rx_buff[0] << 8) | rx_buff[1]) * TMP117_RESOLUTION;
        }


        RESET();
        return ret;
    }

    RetType readTempFahrenheit(float *temp) {
        RESUME();

        RetType ret = CALL(readTempCelsius(temp));
        if (RET_SUCCESS == ret) {
            *temp = celsius_to_fahrenheit(*temp);
        }


        RESET();
        return ret;
    }

    RetType getTempOffset(float *offset) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_TEMP_OFFSET, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *offset = (static_cast<int16_t>(rx_buff[0] << 8) | static_cast<int16_t>(rx_buff[1])) * TMP117_RESOLUTION;
        }

        RESET();
        return ret;
    }

    RetType setTempOffset(float offset) {
        RESUME();

        int16_t resolutionOffset = offset / TMP117_RESOLUTION;
        tx_buff[0] = static_cast<uint8_t>(resolutionOffset >> 8);
        tx_buff[1] = static_cast<uint8_t>(resolutionOffset & 0xFF);

        RetType ret = CALL(writeRegister(TMP117_TEMP_OFFSET, tx_buff, 2));

        RESET();
        return ret;
    }

    RetType getLimit(int16_t *limit, TMP117_HILO_LIMIT_T hilo) {
        RESUME();

        RetType ret = CALL(readRegister(hilo, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *limit = static_cast<int16_t>(rx_buff[0] << 8) | static_cast<int16_t>(rx_buff[1]);
        }

        RESET();
        return ret;
    }

    RetType setLimit(float limit, TMP117_HILO_LIMIT_T hilo) {
        RESUME();

        int16_t resolutionLimit = limit / TMP117_RESOLUTION;
        tx_buff[0] = static_cast<uint8_t>(resolutionLimit >> 8);
        tx_buff[1] = static_cast<uint8_t>(resolutionLimit & 0xFF);

        RetType ret = CALL(writeRegister(hilo, tx_buff, 2));

        RESET();
        return ret;
    }

    RetType isDataReady(bool *dataReady) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *dataReady = ((rx_buff[0] << 8) | rx_buff[1]) & 1 << 13;
        }

        RESET();
        return ret;
    }

    RetType getConfigRegister(uint16_t *configRegister) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *configRegister = (rx_buff[0] << 8) | rx_buff[1];
        }

        RESET();
        return ret;
    }

    RetType getHighLowAlert(uint8_t *alert) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *alert = (rx_buff[0] << 8) | rx_buff[1];
            bitWrite(*alert, 1, bitRead(*alert, 15));
            bitWrite(*alert, 0, bitRead(*alert, 14));
        }

        RESET();
        return ret;
    }

    RetType getAlert(bool *alertResult, TMP117_HILO_ALERT_BIT_T alertBit) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            uint8_t highAlert = bitRead(((rx_buff[0] << 8) | rx_buff[1]), alertBit);
            *alertResult = highAlert == 1;
        }

        RESET();
        return ret;
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
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            uint16_t reset16 = rx_buff[0] << 8 | rx_buff[1];
            bitWrite(reset16, 1, 1);

            tx_buff[0] = static_cast<uint8_t>(reset16 >> 8);
            tx_buff[1] = static_cast<uint8_t>(reset16 & 0xFF);

            ret = CALL(writeRegister(TMP117_CONFIGURATION, tx_buff, 2));
        }

        RESET();
        return ret;
    }

    RetType setContinuousConversionMode(TMP117_MODE_T mode) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, rx_buff, 2));
        if (RET_SUCCESS == ret) {

            uint16_t mode16 = rx_buff[0] << 8 | rx_buff[1];
            switch (mode16) {
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

            tx_buff[0] = static_cast<uint8_t>(mode16 >> 8);
            tx_buff[1] = static_cast<uint8_t>(mode16 & 0xFF);


            ret = CALL(writeRegister(TMP117_CONFIGURATION, tx_buff, 2));
        }

        RESET();
        return ret;
    }

    RetType getConversionMode(TMP117_MODE_T *mode) {
        RESUME();

        static uint16_t *configRegister;
        RetType ret = CALL(getConfigRegister(configRegister));

        if (RET_SUCCESS == ret) {
            uint8_t currMode1 = bitRead(*configRegister, 11);
            uint8_t currMode2 = bitRead(*configRegister, 10);

            if ((currMode1 == 0) && (currMode2 == 1)) {
                *mode = SHUTDOWN;
            } else if ((currMode1 == 1) && (currMode2 == 1)) {
                *mode = ONE_SHOT;
            } else if ((currMode1 == 1) && (currMode2 == 0)) { // Impossible
                ret = RET_ERROR;
            } else { // 0b00 or by default
                *mode = CONTINUOUS_CONVERSION;
            }

        }


        RESET();
        return ret;
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

    RetType readRegister(const uint8_t reg, uint8_t *const data, const size_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C->read(i2cAddr, data, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType writeRegister(const uint8_t reg, uint8_t *const data, const size_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C->write(i2cAddr, data, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }
};

#endif //LAUNCH_CORE_TMP117_H
