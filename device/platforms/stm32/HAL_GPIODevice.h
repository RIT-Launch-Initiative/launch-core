/**
 * HAL Implementation for GPIO Devices
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_HALGPIODEVICE_H
#define LAUNCH_CORE_HALGPIODEVICE_H

#ifdef STM32F446xx
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f446xx.h"
#elif STM32L476xx
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_def.h"
#include "stm32l476xx.h"
#endif

#include "device/GPIODevice.h"
#include "HAL_Handlers.h"
#include "sched/sched.h"
#include "sync/BlockingSemaphore.h"

class HALGPIODevice : public GPIODevice, public CallbackDevice {
public:

    HALGPIODevice(const char *name, GPIO_TypeDef *halGPIO, uint16_t pin) :
                                                            GPIODevice(name),
                                                            m_lock(1),
                                                            m_gpio(halGPIO),
                                                            m_blocked(-1),
                                                            m_pin(pin),
                                                            m_isr_flag(0) {};

    RetType init() override {
        return HALHandlers::register_gpio(m_gpio, this);
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        // disable interrupts to protect access to 'm_isr_flag'
        __disable_irq();

        if (m_isr_flag) {
            // an interrupt occurred

            // reset the flag
            m_isr_flag = 0;

            // re-enable interrupts
            __enable_irq();

            // if a task was blocked waiting for completion of this ISR, wake it up
            if(m_blocked != -1) {
                WAKE(m_blocked);

                // set this task as woken
                m_blocked = -1;
            }
        } else {
            // re-enable interrupts immediately
            __enable_irq();
        }

        return RET_SUCCESS;
    }


    RetType set(uint32_t val) override {
        RESUME();

        if (!(val == 0 || val == 1)) {
            return RET_ERROR;
        }

        RetType ret = CALL(m_lock.acquire());
        if(RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        m_blocked = sched_dispatched;

        HAL_GPIO_WritePin(m_gpio, this->m_pin, static_cast<GPIO_PinState>(val));

        BLOCK();

        ret = CALL(m_lock.release());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) override {
        RESUME();

        RetType ret = CALL(m_lock.acquire());
        if(RET_SUCCESS != ret) {
            RESET();
            return ret;
        }

        m_blocked = sched_dispatched;

        *val = static_cast<int>(HAL_GPIO_ReadPin(m_gpio, m_pin));

        BLOCK();

        ret = CALL(m_lock.release());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    void callback(int) override {
        m_isr_flag = 1;
    }

private:
    BlockingSemaphore m_lock;
    GPIO_TypeDef* m_gpio;
    tid_t m_blocked;
    uint16_t m_pin;
    uint8_t m_isr_flag;
};

#endif //LAUNCH_CORE_HALGPIODEVICE_H
