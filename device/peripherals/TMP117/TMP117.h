/**
 * Platform independent implementation of TMP117 Sensor
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_TMP117_H
#define LAUNCH_CORE_TMP117_H

#include <stdint.h>

#include "device/I2CDevice.h"
#include "sched/macros.h"
#include "return.h"
#include "utils/conversion.h"

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

    /**
     * @brief Initialize the TMP117 sensor
     *
     * @return RetType - Scheduler status code
     */
    RetType init() override {
        RESUME();

        RetType ret = CALL(checkID());
        if (RET_SUCCESS == ret) {
            ret = CALL(softReset());
        }

        RESET();
        return ret;
    }

    /**
     * @brief Get the data from the TMP117 sensor
     *
     * @param data[out] - Pointer to the data struct
     *
     * @return RetType - Scheduler status code
     */
    RetType getData(TMP117_DATA_T *data) {
        RESUME();

        RetType ret = CALL(readTempCelsius(&data->temperature));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }


    /**
     * @brief Get the data from the TMP117 sensor in Celsius
     *
     * @param temp[out] - Pointer to the temperature variable in C
     * @return RetType - Scheduler status code
     *
     */
    RetType readTempCelsius(float *temp) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_TEMP_RESULT, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *temp = (static_cast<int16_t>(rx_buff[0] << 8) | rx_buff[1]) * TMP117_RESOLUTION;
        }

        RESET();
        return ret;
    }


    /**
     * Get the data from the TMP117 sensor in Fahrenheit
     * @param temp[out] - Pointer to the temperature variable in F
     * @return RetType - Scheduler status code
     */
    RetType readTempFahrenheit(float *temp) {
        RESUME();

        RetType ret = CALL(readTempCelsius(temp));
        if (RET_SUCCESS == ret) {
            *temp = celsius_to_fahrenheit(*temp);
        }


        RESET();
        return ret;
    }


    /**
     * Get the temperature offset in mC for calibration
     * @param offset[out - Pointer to the offset variable in mC
     * @return RetType - Scheduler status code
     */
    RetType getTempOffset(float *offset) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_TEMP_OFFSET, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *offset = (static_cast<int16_t>(rx_buff[0] << 8) | static_cast<int16_t>(rx_buff[1])) * TMP117_RESOLUTION;
        }

        RESET();
        return ret;
    }

    /**
     * Set the temperature offset in mC for calibration
     * @param offset[in] - Offset value in mC
     * @return
     */
    RetType setTempOffset(float offset) {
        RESUME();

        int16_t resolutionOffset = offset / TMP117_RESOLUTION;
        tx_buff[0] = static_cast<uint8_t>(resolutionOffset >> 8);
        tx_buff[1] = static_cast<uint8_t>(resolutionOffset & 0xFF);

        RetType ret = CALL(writeRegister(TMP117_TEMP_OFFSET, tx_buff, 2));

        RESET();
        return ret;
    }


    /**
     * @brief Gets the limit set for alerts
     * @param limit[out] - Pointer to the limit variable in C
     * @param hilo[in] - High or Low limit
     * @return
     */
    RetType getLimit(int16_t *limit, TMP117_HILO_LIMIT_T hilo) {
        RESUME();

        RetType ret = CALL(readRegister(hilo, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *limit = static_cast<int16_t>(rx_buff[0] << 8) | static_cast<int16_t>(rx_buff[1]);
        }

        RESET();
        return ret;
    }

    /**
     * @brief Sets a temperature limit for alerts
     * @param limit[in] - Temperature limit in C
     * @param hilo[in] - High or Low limit
     * @return RetType - Scheduler status code
     */
    RetType setLimit(float limit, TMP117_HILO_LIMIT_T hilo) {
        RESUME();

        int16_t resolutionLimit = limit / TMP117_RESOLUTION;
        tx_buff[0] = static_cast<uint8_t>(resolutionLimit >> 8);
        tx_buff[1] = static_cast<uint8_t>(resolutionLimit & 0xFF);

        RetType ret = CALL(writeRegister(hilo, tx_buff, 2));

        RESET();
        return ret;
    }

    /**
     * @brief Gets the current configuration register
     *
     * @param configRegister[out] - Pointer to the configuration register
     * @return RetType - Scheduler status code
     */
    RetType getConfigRegister(uint16_t *configRegister) {
        RESUME();

        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            *configRegister = (rx_buff[0] << 8) | rx_buff[1];
        }

        RESET();
        return ret;
    }

    /**
     * @brief Soft resets the TMP117 sensor
     *
     * @return RetType - Scheduler status code
     */
    RetType softReset() {
        RESUME();
        RetType ret = CALL(readRegister(TMP117_CONFIGURATION, rx_buff, 2));
        if (RET_SUCCESS == ret) {
            uint16_t reset16 = (rx_buff[0] << 8 | rx_buff[1]) | (0b1 << 1);

            tx_buff[0] = static_cast<uint8_t>(reset16 >> 8);
            tx_buff[1] = static_cast<uint8_t>(reset16 & 0xFF);

            ret = CALL(writeRegister(TMP117_CONFIGURATION, tx_buff, 2));
        }

        RESET();
        return ret;
    }

private:
    I2CDevice *mI2C;
    I2CAddr_t i2cAddr;
    uint8_t tx_buff[2]{};
    uint8_t rx_buff[2]{};

    /**
     * @brief Confirms the device ID is correct
     * @return RetType - Scheduler status code
     */
    RetType checkID() {
        RESUME();

        i2cAddr.mem_addr = TMP117_DEVICE_ID;
        RetType ret = CALL(mI2C->read(i2cAddr, rx_buff, 2, 1000));
        if ((RET_SUCCESS == ret) && (DEVICE_ID_VALUE != static_cast<uint16_t>(rx_buff[0] << 8 | rx_buff[1]))) {
            ret = RET_ERROR;
        }

        RESET();
        return ret;
    }

    /**
     * @brief Reads the temperature register and converts it to C
     * @param reg[in] - Register to read
     * @param data[out] - Pointer to the data buffer
     * @param len[in] - Length of the data buffer
     * @return RetType - Scheduler status code
     */
    RetType readRegister(const uint8_t reg, uint8_t *const data, const size_t len) {
        RESUME();

        i2cAddr.mem_addr = reg;
        RetType ret = CALL(mI2C->read(i2cAddr, data, len));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Writes to a register
     * @param reg[in] = Register to write to
     * @param data[out] - Pointer to the data buffer
     * @param len[in] - Length of the data buffer
     * @return RetType - Scheduler status code
     */
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
