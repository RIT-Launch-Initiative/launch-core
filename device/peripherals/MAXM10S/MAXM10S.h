/**
 * @file MAXM10S.h
 * @brief Platform Independent Driver for the MAX-M10S GPS module
 * @details This is the I2C Implementation.
 *          UART and PIO are also supported but are NYI.
 *          Im not even sure if we are going to need them but the option
 *          is there. 🤯
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
 * @brief Platform Independent Driver for the MAX-M10S GPS module
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

   private:
    I2CDevice &mI2C;
    I2CAddr_t addr;
};

#endif  // LAUNCH_CORE_MAXM10S_H