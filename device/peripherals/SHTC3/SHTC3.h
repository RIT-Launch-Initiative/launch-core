/**
 * Platform Independent Driver for the SHTC3 Sensor
 *
 * @author Aaron Chan
 * @author Nate Aquino
 */
#ifndef LAUNCH_CORE_SHTC3_H
#define LAUNCH_CORE_SHTC3_H
#define DEFAULT_SHTC3(X) SHTC3_Readings X = {.id = 16070, .temp = NULL, .humidity = NULL}

#include "device/I2CDevice.h"
#include "return.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "sched/macros/resume.h"
#include "utils/conversion.h"
#include "device/peripherals/SensorDevice.h"

/* The SHTC3 I2C address (8 bits) */
#define SHTC3_I2C_ADDR 0x70



typedef struct 
{
    uint16_t id;
    float temp;
    float humidity;
} SHTC3_Readings

/**
 * @brief The SHTC3 Commands
 *
 */
enum SHTC3_CMD {
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
class SHTC3 : public SensorEncodeDecode{
   public:
    SHTC3(I2CDevice &i2CDevice) : mI2C(i2CDevice), inLowPowerMode(false), addr({.dev_addr = SHTC3_I2C_ADDR << 1, .mem_addr = 0, .mem_addr_size = 2}) {}

    /**
     * @brief Initialize the sensor
     *
     * @return RetType The scheduler status
     */
    RetType init() {
        RESUME();

        RetType ret = CALL(toggleSleep(false));
        if (ret != RET_SUCCESS)
            return ret;

        ret = CALL(reset());
        if (ret != RET_SUCCESS)
            return ret;

        ret = CALL(getID(&this->id));
        if (ret != RET_SUCCESS)
            return ret;

        if ((this->id & 0x083F) != 0x807) {
            return RET_ERROR;
        }

        if (inLowPowerMode) {
            ret = CALL(writeCommand(LOW_POW_MEAS_TEMP));
            SLEEP(1000);
        } else {
            ret = CALL(writeCommand(NORMAL_POW_MEAS_TEMP));
            SLEEP(1000);
        }

        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Get the Humidity And Temp from the sensor
     *
     * @param temperature A pointer to the temperature variable
     * @param humidity A pointer to the humidity variable
     * @return RetType The scheduler status
     */
    RetType getHumidityAndTemp(float *temperature, float *humidity) {
        RESUME();
        static uint16_t rawTemp;
        static uint16_t rawHumidity;

        static uint8_t buffer[2] = {};
        RetType ret = CALL(readCommand(NORMAL_POW_MEAS_HUM_STRETCH, buffer, 2));
        if (ret != RET_SUCCESS)
            return ret;

        rawHumidity = (buffer[0] << 8) | buffer[1];
        *humidity = calcHumidity(rawHumidity);

        ret = CALL(readCommand(NORMAL_POW_MEAS_TEMP_STRETCH, buffer, 2));
        if (ret != RET_SUCCESS)
            return ret;

        rawTemp = (buffer[0] << 8) | buffer[1];
        *temperature = calcTemp(rawTemp);

        RESET();
        return RET_SUCCESS;
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
     * @brief Toggle the sleep mode of the sensor
     *
     * @param setSleep Whether to set the sensor to sleep or not
     * @return RetType The scheduler status
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

        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Reset the sensor
     *
     * @return RetType The scheduler status
     */
    RetType reset() {
        RESUME();

        RetType ret = CALL(writeCommand(RESET_CMD));
        if (ret != RET_SUCCESS) return ret;

        SLEEP(100);

        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Writes a command to the sensor and ignores the response
     *
     * @param command16 The command to write
     * @return RetType The scheduler status
     */
    RetType writeCommand(SHTC3_CMD command16) {
        RESUME();
        addr.dev_addr = (SHTC3_I2C_ADDR << 1);

        uint8_t command8[2];
        uint16ToUint8(command16, command8);
        RetType ret = CALL(mI2C.transmit(addr, command8, 2, 80));
        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Write a command to the sensor, and read the response
     *
     * @param command16 The command to write
     * @param buff The buffer to read into
     * @param numBytes The number of bytes to read
     * @return RetType The scheduler status
     */
    RetType readCommand(SHTC3_CMD command16, uint8_t *buff, uint8_t numBytes) {
        RESUME();
        uint16ToUint8(command16, buff);
        static uint8_t secondAddr = addr.dev_addr;
        secondAddr |= 0x01 << 0;

        RetType ret = CALL(mI2C.transmitReceive(addr, buff, 2, numBytes, 50, secondAddr));
        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Read the ID of the sensor
     *
     * @param id A pointer to the ID variable
     * @return RetType The scheduler status
     */
    RetType getID(uint16_t *id) {
        RESUME();

        static uint8_t data[2] = {};
        RetType ret = CALL(readCommand(READ_ID_CMD, data, 2));
        if (ret != RET_SUCCESS)
            return ret;

        *id = data[0] << 8 | data[1];

        RESET();
        return RET_SUCCESS;
    }

    //    RetType setPowerMode(bool lowPowerMode) {
    //        this->inLowPowerMode = lowPowerMode;
    //    }

   private:
    /* The I2C object */
    I2CDevice &mI2C;
    /* The I2C address of the sensor */
    I2CAddr_t addr;
    /* Is the sensor in low power mode */
    bool inLowPowerMode;
    /* The ID of the sensor */
    uint16_t id;

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

    void encode(void* sensor_struct, uint8_t buffer){
        SHTC3_Readings data = (SHTC3_Readings)sensor_struct;
        uint16_to_uint8(data->id, buffer);
        int32_to_uint8(data->temp, buffer + 2);
        int32_to_uint8(data->humidity, buffer + 6);
    }

    void decode(void* sensor_struct, uint8_t buffer){
        SHTC3_Readings data = (SHTC3_Readings)sensor_struct;
        data->id = uint8_to_int16(buffer);
        data->temp = uint8_to_int64(buffer + 2);
        data->humidity = uint8_to_int64(buffer + 6);
    } 
};

#endif  // LAUNCH_CORE_SHTC3_H
