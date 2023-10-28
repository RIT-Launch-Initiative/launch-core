/**
 * HAL Implementation for GPIO Devices
 *
 * @author Aaron Chan
 * @author Nate Aquino
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
#include "device/platforms/stm32/HAL_Handlers.h"
#include "sched/macros.h"

extern UART_HandleTypeDef huart2;

/// @brief GPIO device controller
class HALGPIODevice : public GPIODevice,
                      public CallbackDevice {
   public:
    /// @brief Create a new HALGPIODevice
    /// @param name the name of the device
    /// @param halGPIO the GPIO port struct pointer (GPIOA, GPIOB, etc)
    /// @param pin the pin number (0-15)
    /// @param exti_pin the pin number to enable external interrupts on. If -1 exti is disabled.
    HALGPIODevice(const char *name, GPIO_TypeDef *halGPIO, uint16_t pin, uint16_t exti_pin)
        : GPIODevice(name),
          m_gpio(halGPIO),
          m_pin(pin),
          m_isr_flag(0),
          m_blocked(-1),
          m_exti_en(exti_pin <= -1 ? 0 : 1),
          m_exti_pin(exti_pin){};

    RetType init() override {
        HAL_UART_Transmit(&huart2, (uint8_t *)"GPIO INIT\r\n", 11, 100);
        HAL_UART_Transmit(&huart2, (uint8_t *)"EXTI: ", 6, 100);
        HAL_UART_Transmit(&huart2, (uint8_t *)(m_exti_en ? "ENABLED\r\n" : "DISABLED\r\n"), 10, 100);
        if (!m_exti_en)
            return RET_SUCCESS;  // exti disabled so we dont need to init
        else {
            // exti is enabled for this device, we need to register it.
            // num is 0 right now because im not sure how it could be used.
            RetType ret = HALHandlers::register_gpio_exti(m_exti_pin, this, 0);
            HAL_UART_Transmit(&huart2, (uint8_t *)"EXTI REGISTER: ", 15, 100);
            HAL_UART_Transmit(&huart2, (uint8_t *)(ret == RET_SUCCESS ? "SUCCESS\r\n" : "ERROR\r\n"), 9, 100);
            if (ret != RET_SUCCESS)
                return ret;  // error out if we failed to register

            return RET_SUCCESS;
        }
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        RESUME();
        if (!m_exti_en)
            return RET_SUCCESS;  // exti disabled, no need to poll
        else {
            // aquire lock on isr flag by disabling interrupts
            __disable_irq();

            // check if the isr flag is set
            if (m_isr_flag) {
                HAL_UART_Transmit(&huart2, (uint8_t *)"GPIO ISR\r\n", 10, 100);
                // an interrupt occured! Reset the flag
                m_isr_flag = false;

                // re-enable interrupts now that we are done reading the flag
                __enable_irq();

                // if a task was blocked waiting for completion of this interrupt, wake it up
                if (m_blocked != -1) {
                    HAL_UART_Transmit(&huart2, (uint8_t *)"GPIO WAKE\r\n", 11, 100);
                    // wake macro wakes up the task with the given tid (task id)
                    WAKE(m_blocked);
                    // mark this task as no longer blocked
                    m_blocked = -1;
                }
            } else
                __enable_irq();  // re-enable interrupts if no interrupt occured

            RESET();
            return RET_SUCCESS;
        }
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

    RetType registerBlock(tid_t task_block) {
        if (m_blocked == -1) {
            m_blocked = task_block;
            return RET_SUCCESS;
        }
        return RET_ERROR;
    }

    /// @brief called by the GPIO interrupt handler asynchronously
    void callback(int) {
        HAL_UART_Transmit(&huart2, (uint8_t *)"GPIO CALLBACK\r\n", 15, 100);
        // set isr flag to true
        m_isr_flag = true;
    }

   private:
    /// @brief The GPIO port struct ptr
    GPIO_TypeDef *m_gpio;

    /// @brief The GPIO pin number
    uint16_t m_pin;

    /// @brief The Interrupt Service Routine flag
    uint8_t m_isr_flag;

    /// @brief The currently blocked task
    tid_t m_blocked;

    /// @brief The external interrupt enable flag
    uint8_t m_exti_en;

    /// @brief The external interrupt pin number
    uint16_t m_exti_pin;
};

#endif  // LAUNCH_CORE_HALGPIODEVICE_H
