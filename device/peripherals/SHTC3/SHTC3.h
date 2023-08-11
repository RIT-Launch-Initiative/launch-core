/**
 * Platform Independent Driver for the SHTC3 Sensor
 *
 * @author Aaron Chan
 * @author Nate Aquino
 */
#ifndef LAUNCH_CORE_SHTC3_H
#define LAUNCH_CORE_SHTC3_H
#define SHTC3_DATA_STRUCT(variable_name) SHTC3_DATA_T variable_name = {.id = 16000, .temperature = 0, .humidity = 0}

#include "device/I2CDevice.h"
#include "return.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "sched/macros/resume.h"

/* The SHTC3 I2C address (8 bits) */
#define SHTC3_I2C_ADDR 0x70


using SHTC3_DATA_T = struct {
    const uint16_t id;
    float temperature;
    float humidity;
};

/**
 * @brief The SHTC3 Commands
 *
 */
using SHTC3_CMD = enum {
    /* Sleep command */
    SLEEP_CMD = 0xB098,
    /* Wakeup command */
    WAKEUP_CMD = 0x3517,
    /* Soft reset command */
    RESET_CMD = 0x805D,

    /* Read ID */
    READ_ID_CMD = 0xEFC8,

    /* Normal power measure temperature */
    NORMAL_POW_MEAS_TEMP = 0x7866,
    /* Low power measure temperature */
    LOW_POW_MEAS_TEMP = 0x609C,
    /* Normal power measure humidity */
    NORMAL_POW_MEAS_HUM = 0x58E0,
    /* Low power measure humidity */
    LOW_POW_MEAS_HUM = 0x401A,

    /* Normal power measure temperature with clock stretching */
    NORMAL_POW_MEAS_TEMP_STRETCH = 0x7CA2,
    /* Low power measure temperature with clock stretching */
    LOW_POW_MEAS_TEMP_STRETCH = 0x6458,
    /* Normal power measure humidity with clock stretching */
    NORMAL_POW_MEAS_HUM_STRETCH = 0x5C24,
    /* Low power measure humidity with clock stretching */
    LOW_POW_MEAS_HUM_STRETCH = 0x44DE,
};

/**
 * @brief Platform Independent Driver for the SHTC3 Sensor
 *
 */
class SHTC3 : public Device {
public:
    explicit SHTC3(I2CDevice &i2CDevice, uint16_t address = SHTC3_I2C_ADDR, const char *name = "SHTC3")
            : Device(name), m_i2c(i2CDevice), m_isLowPower(false),
              m_i2cAddr({.dev_addr = static_cast<uint16_t>(address << 1), .mem_addr = 0, .mem_addr_size = 2}) {}

    /**
     * @brief Initialize the sensor
     *
     * @return Scheduler Status
     */
    RetType init() {
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
    uint16_t id;
    uint8_t m_buff[4];

    /**
     * @brief Perform a 16 bit to 8 bit conversion
     *
     * @param data16 The 16 bit data (should be 2 bytes)
     * @param data8 The 8 bit data buffer
     */
    void uint16ToUint8(uint16_t data16, uint8_t *data8) {
        data8[0] = static_cast<uint8_t>(data16 >> 8);
        data8[1] = static_cast<uint8_t>(data16 & 0xFF);
    }

    /**
     * @brief Writes a command to the sensor and ignores the response
     *
     * @param command16 The command to write
     * @return Scheduler Status
     */
    RetType writeCommand(SHTC3_CMD command16) {
        RESUME();
        m_i2cAddr.dev_addr = (SHTC3_I2C_ADDR << 1);

        static uint8_t command8[2];
        uint16ToUint8(command16, command8);
        RetType ret = CALL(m_i2c.transmit(m_i2cAddr, command8, 2, 80));

        RESET();
        return ret;
    }

    /**
     * @brief Write a command to the sensor, and read the response
     *
     * @param command16 The command to write
     * @param buff The buffer to read into
     * @param numBytes The number of bytes to read
     * @return Scheduler Status
     */
    RetType readCommand(SHTC3_CMD command16, uint8_t *buff, uint8_t numBytes) {
        RESUME();
        m_i2cAddr.dev_addr = (SHTC3_I2C_ADDR << 1);

        RetType ret = CALL(m_i2c.transmit(m_i2cAddr, reinterpret_cast<uint8_t *>(&command16), 2, 50));
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
