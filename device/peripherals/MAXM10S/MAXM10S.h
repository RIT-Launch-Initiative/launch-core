/**
 * @file MAXM10S.h
 * @brief Platform Independent Driver for the MAX-M10S GPS module
 * @details This is the I2C Implementation.
 *          UART and PIO are also supported but are NYI.
 *          Im not even sure if we are going to need them but the option
 *          is there. 🤯. Also note this sensor is not writeable with
 *          2 exceptions. Those being writing NMEA and UBX messages.
 *          The datasheet is not very clear on how to do this.
 * @author Nate Aquino
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
    MAXM10S(I2CDevice &i2CDevice)
        : mI2C(i2CDevice),
          addr({
              .dev_addr = MAXM10S_I2C_ADDR << 1,
              .mem_addr = 0,
              .mem_addr_size = 2,
          }) {}

    /**
     * @brief Initialize the MAXM10S sensor
     *
     * @return RetType the scheduler status
     */
    RetType init() {
        RESUME();
        RESET();
        return RET_SUCCESS;  /// @todo NYI
    }

    /**
     * @brief Reads the amount of data available from the sensor
     * @details This is a random access read (register read)
     * @param buff the gps data buffer
     * @param numBytes the number of bytes to read
     * @return RetType the scheduler status
     */
    RetType readDataRandAccess(uint8_t *buff, int *readBytes) {
        RESUME();

        // read the amount of data available
        uint8_t byteCount[2];
        RetType ret = CALL(readRegister(BYTE_COUNT_HIGH, byteCount, 1));
        if (ret != RET_SUCCESS)
            return ret;
        ret = CALL(readRegister(BYTE_COUNT_HIGH, byteCount + 1, 1));
        if (ret != RET_SUCCESS)
            return ret;
        *readBytes = (byteCount[0] << 8) | byteCount[1];
        if (*readBytes == 0)
            return RET_SUCCESS;  // no data available

        // read the data
        ret = CALL(readRegister(DATA_STREAM, buff, *readBytes));
        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
    }

    /**
     * @brief Reads the amount of data available from the sensor
     * @details This is a current access read (TXRX read)
     * @param buff the gps data buffer
     * @return RetType the scheduler status
     */
    RetType readDataCurrentAccess(uint8_t *buff) {
        RESUME();
        RESET();
        return RET_SUCCESS;  /// @todo NYI
    }

    /**
     * @brief Reads a register from the MAXM10S sensor
     *
     * @param reg the register to read from
     * @param buff the buffer to read into
     * @param numBytes the number of bytes to read
     * @return RetType the scheduler status
     */
    RetType readRegister(enum MAXM10S_REG reg, uint8_t *buff, int numBytes) {
        RESUME();

        addr.mem_addr = reg;
        RetType ret = CALL(mI2C.read(addr, buff, numBytes, 1500));  // sensor timeout
        if (ret != RET_SUCCESS)
            return ret;

        RESET();
        return RET_SUCCESS;
    }

   private:
    /* The I2C object */
    I2CDevice &mI2C;
    /* The I2C address of the sensor */
    I2CAddr_t addr;
};

#endif  // LAUNCH_CORE_MAXM10S_H