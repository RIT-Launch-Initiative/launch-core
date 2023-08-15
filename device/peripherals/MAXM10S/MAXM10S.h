/**
 * @file MAXM10S.h
 * @brief Platform Independent Driver for the MAX-M10S GPS module
 * @details Supports both I2C and UART interfaces
 *
 * @author Aaron Chan
 * @author Nate Aquino
 *
 *
 * @link https://content.u-blox.com/sites/default/files/MAX-M10S_IntegrationManual_UBX-20053088.pdf
 */
#ifndef MAXM10S_H
#define MAXM10S_H

#include "return.h"
#include "sched/macros.h"
#include "device/StreamDevice.h"
#include "device/I2CDevice.h"
#include "device/GPIODevice.h"

class MAXM10S : public Device {
public:
    static const uint8_t MAXM10S_I2C_ADDR = 0x42;

    typedef enum {
        /* The hight byte of the amount of data available from the sensor */
        BYTE_COUNT_HIGH = 0xFD,
        /* The low byte of the amount of data available from the sensor */
        BYTE_COUNT_LOW = 0xFE,
        /* The data stream register */
        DATA_STREAM = 0xFF
    } MAXM10S_REG;

    /**
     * @brief CTOR For MAXM10S
     * @param i2c_device the I2C device to use
     */
    MAXM10S(I2CDevice &i2c_device, StreamDevice &stream_device, GPIODevice &reset_pin, GPIODevice &interrupt_pin, const char *device_name = "MAXM10S") :
            Device(device_name), m_i2c(i2c_device), m_stream(stream_device), m_reset_pin(reset_pin),
            m_interrupt_pin(interrupt_pin) {};

    RetType init() override {
        RESUME();

        RetType ret = CALL(reset());
        if (RET_SUCCESS == ret) {
            ret = CALL(m_interrupt_pin.set(0));
        }

        RESET();
        return ret;
    }

    /**
     * @brief Reads the amount of data available from the sensor
     * @details This is a random access read (all at once)
     * @param buff the gps data buffer
     * @param buff_len the length of the buffer
     * @param bytes_available pointer to the number of bytes available to be read
     * @return Scheduler Status
     */
    RetType read_data_rand_access(uint8_t *buff, size_t buff_len, size_t *bytes_available) {
        RESUME();

        RetType ret = CALL(get_amt_data(bytes_available));
        if (*bytes_available == 0 || *bytes_available > buff_len) { // No data available
            ret = RET_ERROR;
        } else {
            ret = CALL(read_reg(DATA_STREAM, buff, *bytes_available));
        }

        RESET();
        return ret;
    }

    /**
     * @brief Get the amount of data available from the sensor
     *
     * @param amt ptr to the amount of data available
     * @return Scheduler Status
     */
    RetType get_amt_data(size_t *amt) {
        RESUME();

        RetType ret = CALL(read_reg(BYTE_COUNT_HIGH, m_buff, 2));
        *amt = (m_buff[0] << 8) | m_buff[1];

        RESET();
        return ret;
    }

    /**
     * @brief Reads the amount of data available from the sensor
     * @details This is a current access read (stream read)
     * @param buff the gps data buffer
     * @return Scheduler Status
     */
    RetType read_data_curr_access(uint8_t *buff) {  // TODO: Untested
        RESUME();

        RetType ret;

        addr.mem_addr = DATA_STREAM;

        do {
            ret = CALL(m_i2c.read(addr, buff, 1, 1500));
            if (ret == RET_SUCCESS) buff++;
        } while (ret != RET_ERROR || *(buff - 1) != 0xFF);

        RESET();
        return ret;
    }


    /**
     * @brief Requests and reads positional data
     * @details This is a current access read (stream read)
     * @param buff the buffer to read data into
     * @return Scheduler Status
     */
    RetType uart_read_posllh_data(uint8_t *buff) {
        RESUME();

        static constexpr uint8_t posllh[] = {0xB5, 0x62, 0x01, 0x21, 0x00, 0x00, 0x22, 0x67};
        RetType ret = CALL(m_stream.write(const_cast<uint8_t *>(posllh), 8));
        if (RET_SUCCESS != ret) {
            RESET();
            return ret;
        };

        ret = CALL(m_stream.read(buff, 28));

        RESET();
        return ret;
    }

    RetType reset() {
        RESUME();
        CALL(m_reset_pin.set(0));
        SLEEP(10);
        CALL(m_reset_pin.set(1));
        RESET();
        return RET_SUCCESS;
    }

private:
    /* The I2C object */
    I2CDevice &m_i2c;
    StreamDevice &m_stream;
    GPIODevice &m_reset_pin;
    GPIODevice &m_interrupt_pin;
    I2CAddr_t addr = {.dev_addr = MAXM10S_I2C_ADDR << 1, .mem_addr = 0, .mem_addr_size = 1};
    uint8_t m_buff[2];

    /**
     * @brief Reads a register from the MAXM10S sensor
     *
     * @param reg the register to read from
     * @param buff the buffer to read into
     * @param numBytes the number of bytes to read
     * @return Scheduler Status
     */
    RetType read_reg(MAXM10S_REG reg, uint8_t *buff, size_t numBytes) {
        RESUME();

        addr.mem_addr = 0;
        RetType ret = CALL(m_i2c.transmit(addr, (uint8_t *) reg, 1, 150));
        if (RET_SUCCESS != ret) goto read_reg_end;

        addr.mem_addr = reg;
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
     * @return Scheduler Status
     */
    RetType sendCommand(uint8_t *cmdBuff, int cmdLen) {
        RESUME();

        RetType ret = CALL(m_i2c.transmit(addr, cmdBuff, cmdLen, 50));
        if (RET_SUCCESS != ret)
            return ret;

        RESET();
        return RET_SUCCESS;
    }
};

#endif  // MAXM10S_H
