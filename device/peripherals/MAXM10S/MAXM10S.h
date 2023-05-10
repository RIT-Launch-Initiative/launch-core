/**
 * @file MAXM10S.h
 * @brief Platform Independent Driver for the MAX-M10S GPS module
 * @details This is the I2C Implementation.
 *          UART and PIO are also supported but are NYI.
 *
 *          https://content.u-blox.com/sites/default/files/MAX-M10S_IntegrationManual_UBX-20053088.pdf
 *
 *          Im not even sure if we are going to need them but the option
 *          is there. 🤯. Also note this sensor is not writeable with
 * @author Nate Aquino
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_MAXM10S_H
#define LAUNCH_CORE_MAXM10S_H

/// @note Documentation for this device lives here:
/// @link https://content.u-blox.com/sites/default/files/MAX-M10S_IntegrationManual_UBX-20053088.pdf

#include "device/I2CDevice.h"
#include "return.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "sched/macros/resume.h"

/* The MAXM10S (default) I2C address (8 bits) */
#define MAXM10S_I2C_ADDR 0x42

/**
 * @brief The MAXM10S Registers
 *
 */
enum MAXM10S_REG {
    /* The hight byte of the amount of data available from the sensor */
    BYTE_COUNT_HIGH = 0xFD,
    /* The low byte of the amount of data available from the sensor */
    BYTE_COUNT_LOW = 0xFE,
    /* The data stream register */
    DATA_STREAM = 0xFF
};

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
     * @param numBytes the number of bytes to read
     * @return RetType the scheduler status
     */
    RetType read_data_rand_access(uint8_t *buff, int *readBytes) {  /// @todo TESTME!
        RESUME();

        RetType ret = CALL(get_amt_data(readBytes));
        if (*readBytes == 0)
            return RET_SUCCESS;  // no data available

        // read the data
        ret = CALL(read_reg(DATA_STREAM, buff, *readBytes));
        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
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

        RetType ret = CALL(read_reg(BYTE_COUNT_HIGH, byteCount, 1));
        if (ret != RET_SUCCESS) goto get_amt_data_end;

        ret = CALL(read_reg(BYTE_COUNT_HIGH, byteCount + 1, 1));
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
    RetType read_data_curr_access(uint8_t *buff) {  /// @todo TESTME!
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


    RetType read_uart(uint8_t *buff, size_t len) {
        RESUME();
        RESET();
        return RET_SUCCESS;
    }

    RetType read_i2c(uint8_t *buff, size_t len) {
        RESUME();

        RetType ret =

        RESET();
        return RET_SUCCESS;
    }

    RetType reset() {
        RESUME();
        CALL(reset_pin.set(0));
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
    I2CAddr_t addr = {.dev_addr = MAXM10S_I2C_ADDR << 1, .mem_addr = 0, .mem_addr_size = 2};
    using m_read = RetType (*)(uint8_t *buff, size_t len);

    /**
     * @brief Reads a register from the MAXM10S sensor
     *
     * @param reg the register to read from
     * @param buff the buffer to read into
     * @param numBytes the number of bytes to read
     * @return RetType the scheduler status
     */
    RetType read_reg(enum MAXM10S_REG reg, uint8_t *buff, int numBytes) {  /// @todo TESTME!
        RESUME();

        addr.mem_addr = reg;
        RetType ret = CALL(m_i2c.read(addr, buff, numBytes, 1500));  // sensor timeout
        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
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