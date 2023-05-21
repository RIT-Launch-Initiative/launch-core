/**
 * @file MAXM10S.h
 * @brief Platform Independent Driver for the MAX-M10S GPS module
 * @details This is the I2C Implementation.
 *          UART and PIO are also supported but are NYI.
 *
 *
 *          Im not even sure if we are going to need them but the option
 *          is there. 🤯. Also note this sensor is not writeable with
 * @author Nate Aquino
 * @author Aaron Chan
 *
 *
 * @link https://content.u-blox.com/sites/default/files/MAX-M10S_IntegrationManual_UBX-20053088.pdf
 */
#ifndef LAUNCH_CORE_MAXM10S_H
#define LAUNCH_CORE_MAXM10S_H

#include "device/I2CDevice.h"
#include "return.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "sched/macros/resume.h"
#include "stm32f4xx_hal_i2c.h"

extern I2C_HandleTypeDef hi2c1;


/* The MAXM10S (default) I2C address (8 bits) */
#define MAXM10S_I2C_ADDR 0x42

/**
 * @brief The MAXM10S Registers
 *
 */
typedef enum {
    /* The hight byte of the amount of data available from the sensor */
    BYTE_COUNT_HIGH = 0xFD,
    /* The low byte of the amount of data available from the sensor */
    BYTE_COUNT_LOW = 0xFE,
    /* The data stream register */
    DATA_STREAM = 0xFF
} MAXM10S_REG;

/**
 * @brief Platform Independent Driver for the MAXM10S GPS module
 */
class MAXM10S {
public:
    /**
     * @brief CTOR For MAXM10S
     * @param i2CDevice the I2C device to use
     */
    MAXM10S(I2CDevice &i2CDevice, StreamDevice &streamDevice, GPIODevice &resetPin) : m_i2c(i2CDevice), m_stream(streamDevice), reset_pin(resetPin) {};

    /**
     * @brief Initialize the MAXM10S sensor
     *
     * @return RetType the scheduler status
     */
    RetType init() {
        RESUME();

        RetType ret = CALL(reset());
        if (ret != RET_SUCCESS) goto init_end;

        init_end:
        RESET();
        return RET_SUCCESS;  /// @todo NYI
    }

    /**
     * @brief Reads the amount of data available from the sensor
     * @details This is a random access read (all at once)
     * @param buff the gps data buffer
     * @param buff_len the length of the buffer
     * @param bytes_available pointer to the number of bytes available to be read
     * @return RetType the scheduler status
     */
    RetType read_data_rand_access(uint8_t *buff, size_t buff_len, size_t *bytes_available) {
        RESUME();


        RetType ret = CALL(get_amt_data(bytes_available));
        if (*bytes_available == 0 || *bytes_available > buff_len) {
            RESET();
            return RET_ERROR;  // no data available or buffer too small
        }

        // read the data
        ret = CALL(read_reg(DATA_STREAM, buff, *bytes_available));

        RESET();
        return ret;
    }

    /**
     * @brief Get the amount of data available from the sensor
     *
     * @param amt ptr to the amount of data available
     * @return RetType the scheduler status
     */
    RetType get_amt_data(size_t *amt) {
        RESUME();

        // read the amount of data available
        static uint8_t byteCount[2];

        RetType ret = CALL(read_reg(BYTE_COUNT_HIGH, byteCount, 2));
        if (ret != RET_SUCCESS) goto get_amt_data_end;

        *amt = (byteCount[0] << 8) | byteCount[1];

        get_amt_data_end:
        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Reads the amount of data available from the sensor
     * @details This is a current access read (stream read)
     * @param buff the gps data buffer
     * @return RetType the scheduler status
     */
    RetType read_data_curr_access(uint8_t *buff) {  // TODO: Untested
        RESUME();

        RetType ret;

        addr.mem_addr = DATA_STREAM;
        // read as much data as possible (assuming RET_ERROR means NACK?)
        // I know for sure 0xFF is the end of the data stream
        do {
            ret = CALL(m_i2c.read(addr, buff, 1, 1500));  // sensor timeout
            if (ret == RET_SUCCESS) buff++;
        } while (ret != RET_ERROR || *(buff - 1) != 0xFF);

        RESET();
        return ret;
    }


    /**
     * @brief Requests and reads positional data
     * @details This is a current access read (stream read)
     * @param buff the buffer to read data into
     * @return RetType the scheduler status
     */
    RetType uart_read_posllh_data(uint8_t *buff) {
        RESUME();

        static constexpr uint8_t posllh[] = {0xB5, 0x62, 0x01, 0x21, 0x00, 0x00, 0x22, 0x67};
        RetType ret = CALL(m_stream.write(const_cast<uint8_t *>(posllh), 8));
        if (ret != RET_SUCCESS) goto uart_read_posllh_data_end;

        ret = CALL(m_stream.read(buff, 28));

        uart_read_posllh_data_end:
        RESET();
        return ret;
    }

    RetType reset() {
        RESUME();
        CALL(reset_pin.set(0));
        SLEEP(10);
        CALL(reset_pin.set(1));
        RESET();
        return RET_SUCCESS;
    }

private:
    /* The I2C object */
    I2CDevice &m_i2c;
    StreamDevice &m_stream;
    GPIODevice &reset_pin;
    /* The I2C address of the sensor */
    I2CAddr_t addr = {.dev_addr = MAXM10S_I2C_ADDR << 1, .mem_addr = 0, .mem_addr_size = 1};

    /**
     * @brief Reads a register from the MAXM10S sensor
     *
     * @param reg the register to read from
     * @param buff the buffer to read into
     * @param numBytes the number of bytes to read
     * @return RetType the scheduler status
     */
    RetType read_reg(MAXM10S_REG reg, uint8_t *buff, size_t numBytes) {  /// @todo TESTME!
        RESUME();

        addr.mem_addr = 0;
        RetType ret = CALL(m_i2c.transmit(addr, (uint8_t *) reg, 1, 150));
        if (ret != RET_SUCCESS) goto read_reg_end;

        addr.mem_addr = static_cast<uint8_t>(reg);
        ret = CALL(m_i2c.read(addr, buff, numBytes, 1500));

        read_reg_end:
        RESET();
        return ret;
    }

    /**
     * @brief Writes commands to the MAXM10S sensor
     *
     * @param cmdBuff The NMBA or UBX command to send
     * @param cmdLen The length of the command
     * @return RetType the scheduler status
     */
    RetType sendCommand(uint8_t *cmdBuff, int cmdLen) {  /// @todo TESTME!
        RESUME();

        RetType ret = CALL(m_i2c.transmit(addr, cmdBuff, cmdLen, 50));
        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
    }
};

#endif  // LAUNCH_CORE_MAXM10S_H
