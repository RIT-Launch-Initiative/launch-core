/**
 * @file MAXM10S.h
 * @brief Platform Independent Driver for the MAX-M10S GPS module
 *
 * @author Nate Aquino
 */
#ifndef LAUNCH_CORE_MAXM10S_H
#define LAUNCH_CORE_MAXM10S_H

#pragma region Includes

#include "device/I2CDevice.h"
#include "return.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "sched/macros/resume.h"

#pragma endregion

#define MAXM10S_I2C_ADDR 0x42

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