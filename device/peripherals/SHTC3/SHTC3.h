/**
 * Platform Independent Driver for the SHTC3 Sensor
 *
 * @author Aaron Chan
 * @author Nate Aquino
 *
 * @note As of Aug 11, 2023 there should be no more software support unless the sensor is used in a new project
 */

#ifndef LAUNCH_CORE_SHTC3_H
#define LAUNCH_CORE_SHTC3_H
#define SHTC3_DATA_STRUCT(variable_name) SHTC3::SHTC3_DATA_T variable_name = {.id = 16000, .temperature = 0, .humidity = 0}

#include "device/I2CDevice.h"
#include "sched/macros.h"
#include "return.h"

class SHTC3 : public Device {
public:
    static constexpr uint8_t SHTC3_I2C_ADDR = 0x70;

    typedef struct {
        const uint16_t id;
        float temperature;
        float humidity;
    } SHTC3_DATA_T;

    typedef enum {
        SLEEP_CMD = 0xB098,
        WAKEUP_CMD = 0x3517,
        RESET_CMD = 0x805D,
        READ_ID_CMD = 0xEFC8,
        NORMAL_POW_MEAS_TEMP = 0x7866,
        LOW_POW_MEAS_TEMP = 0x609C,
        NORMAL_POW_MEAS_HUM = 0x58E0,
        LOW_POW_MEAS_HUM = 0x401A,
        NORMAL_POW_MEAS_TEMP_STRETCH = 0x7CA2,
        LOW_POW_MEAS_TEMP_STRETCH = 0x6458,
        NORMAL_POW_MEAS_HUM_STRETCH = 0x5C24,
        LOW_POW_MEAS_HUM_STRETCH = 0x44DE,
    } SHTC3_CMD_T;

    explicit SHTC3(I2CDevice &i2CDevice, const char *name = "SHTC3") : Device(name), m_i2c(i2CDevice), m_isLowPower(false),
              m_i2cAddr({.dev_addr = static_cast<uint16_t>(SHTC3_I2C_ADDR << 1), .mem_addr = 0, .mem_addr_size = 2}) {}

    /**
     * @brief Initialize the sensor
     *
     * @return Scheduler Status
     */
    RetType init() override {
        RESUME();

        RetType ret = CALL(toggleSleep(false));
        ERROR_CHECK(ret);

        ret = CALL(reset());
        ERROR_CHECK(ret);

        ret = CALL(checkID());
        ERROR_CHECK(ret);

        if (m_isLowPower) {
            ret = CALL(writeCommand(LOW_POW_MEAS_TEMP));
        } else {
            ret = CALL(writeCommand(NORMAL_POW_MEAS_TEMP));
        }

        SLEEP(1000);
        RESET();
        return ret;
    }

    RetType getData(SHTC3_DATA_T *data) {
        RESUME();

        RetType ret = CALL(getHumidityAndTemp(&data->temperature, &data->humidity));

        RESET();
        return ret;
    }

    /**
     * @brief Get the Humidity And Temp from the sensor
     *
     * @param temperature A pointer to the temperature variable
     * @param humidity A pointer to the humidity variable
     * @return Scheduler Status
     */
    RetType getHumidityAndTemp(float *temperature, float *humidity) {
        RESUME();

        RetType ret = CALL(readCommand(NORMAL_POW_MEAS_HUM_STRETCH, m_buff, 2));
        if (RET_SUCCESS == ret) {
            *humidity = calcHumidity((m_buff[0] << 8) | m_buff[1]);
        }

        ret = CALL(readCommand(NORMAL_POW_MEAS_TEMP_STRETCH, m_buff, 2));
        if (RET_SUCCESS == ret) {
            *temperature = calcTemp((m_buff[0] << 8) | m_buff[1]);
        }

        RESET();
        return RET_SUCCESS;
    }


    /**
     * @brief Toggle the sleep mode of the sensor
     *
     * @param setSleep Whether to set the sensor to sleep or not
     * @return Scheduler Status
     */
    RetType toggleSleep(bool setSleep) {
        RESUME();

        RetType ret;
        if (setSleep) {
            ret = CALL(writeCommand(SLEEP_CMD));
        } else {
            ret = CALL(writeCommand(WAKEUP_CMD));
            SLEEP(1);
        }

        RESET();

        return ret;
    }

    /**
     * @brief Reset the sensor
     *
     * @return Scheduler Status
     */
    RetType reset() {
        RESUME();

        RetType ret = CALL(writeCommand(RESET_CMD));
        SLEEP(100);

        RESET();
        return ret;
    }

    RetType poll() override {
        return RET_SUCCESS;
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

private:
    /* The I2C object */
    I2CDevice &m_i2c;
    /* The I2C address of the sensor */
    I2CAddr_t m_i2cAddr;
    /* Is the sensor in low power mode */
    bool m_isLowPower;
    /* The ID of the sensor */
    uint8_t m_buff[4];

    /**
     * @brief Writes a command to the sensor and ignores the response
     *
     * @param command The command to write
     * @return Scheduler Status
     */
    RetType writeCommand(SHTC3_CMD_T command) {
        RESUME();

        m_buff[0] = command >> 8;
        m_buff[1] = command & 0xFF;
        m_i2cAddr.dev_addr = (SHTC3_I2C_ADDR << 1);
        RetType ret = CALL(m_i2c.transmit(m_i2cAddr, m_buff, 2, 80));

        RESET();
        return ret;
    }

    /**
     * @brief Write a command to the sensor, and read the response
     *
     * @param command The command to write
     * @param buff The buffer to read into
     * @param numBytes The number of bytes to read
     * @return Scheduler Status
     */
    RetType readCommand(SHTC3_CMD_T command, uint8_t *buff, uint8_t numBytes) {
        RESUME();

        m_i2cAddr.dev_addr = (SHTC3_I2C_ADDR << 1);
        RetType ret = CALL(m_i2c.transmit(m_i2cAddr, reinterpret_cast<uint8_t *>(&command), 2, 50));
        ERROR_CHECK(ret);

        m_i2cAddr.dev_addr = (SHTC3_I2C_ADDR << 1) | 0x01;
        ret = CALL(m_i2c.receive(m_i2cAddr, buff, numBytes, 50));

        RESET();
        return ret;
    }

    /**
     * @brief Check the sensor ID is correct
     *
     * @return Scheduler Status
     */
    RetType checkID() {
        RESUME();

        RetType ret = CALL(readCommand(READ_ID_CMD, m_buff, 2));
        ERROR_CHECK(ret);

        if (0x807 != ((m_buff[0] << 8 | m_buff[1]) & 0x083F)) ret = RET_ERROR;

        RESET();
        return ret;
    }

    /**
     * @brief Calculate the temperature from the raw value
     *
     * @param rawValue  The raw value from the sensor
     * @return float  The temperature
     */
    static float calcTemp(uint16_t rawValue) {
        return 175 * static_cast<float>(rawValue) / 65536.0f - 45.0f;
    }

    /**
     * @brief Calculate the humidity from the raw value
     *
     * @param rawValue The raw value from the sensor
     * @return float The humidity
     */
    static float calcHumidity(uint16_t rawValue) {
        return 100 * static_cast<float>(rawValue) / 65536.0f;
    }

    /**
     * @brief Calculate the CRC8 checksum of the data
     *
     * @param data the data to calculate the checksum of
     * @param len the length of the data
     * @return uint8_t The checksum
     */
    static uint8_t crc8(const uint8_t *data, int len) {
        const uint8_t POLYNOMIAL(0x31);
        uint8_t crc(0xFF);

        for (int i = len; i; --i) {
            crc ^= *data++;

            for (int j = 8; j; --j) {
                crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
            }
        }
        return crc;
    }

};

#endif  // LAUNCH_CORE_SHTC3_H
