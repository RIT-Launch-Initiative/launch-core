/**
 * HAL Implementation for GPIO Devices
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_HALGPIODEVICE_H
#define LAUNCH_CORE_HALGPIODEVICE_H

#ifdef STM32F446xx
#include "stm32f446xx.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#elif STM32L476xx
#include "stm32l476xx.h"
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_hal_gpio.h"
#endif

#include "device/GPIODevice.h"
#include "sched/sched.h"

class HALGPIODevice : public GPIODevice {
   public:
    HALGPIODevice(const char *name, GPIO_TypeDef *halGPIO, uint16_t pin) : GPIODevice(name),
                                                                           m_gpio(halGPIO),
                                                                           m_pin(pin){};

    RetType init() override {
        return RET_SUCCESS;
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        return RET_SUCCESS;
    }

    RetType set(uint32_t val) override {
        if (!(val == 0 || val == 1)) {
            return RET_ERROR;
        }

        // No need to check return, this is a void
        HAL_GPIO_WritePin(m_gpio, this->m_pin, static_cast<GPIO_PinState>(val));

        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) override {
        *val = static_cast<int>(HAL_GPIO_ReadPin(m_gpio, m_pin));

        return RET_SUCCESS;
    }

   private:
    GPIO_TypeDef *m_gpio;
    uint16_t m_pin;
};

#endif  // LAUNCH_CORE_HALGPIODEVICE_H
