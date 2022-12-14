/**
 * HAL Implementation for GPIO Devices
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_HALGPIODEVICE_H
#define LAUNCH_CORE_HALGPIODEVICE_H

#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f446xx.h"


#include "device/GPIODevice.h"
#include "HAL_Handlers.h"
#include "sched/sched.h"
#include "sync/BlockingSemaphore.h"

class HALGPIODevice : public GPIODevice, public CallbackDevice {
public:

    HALGPIODevice(const char *name, GPIO_TypeDef *halGPIO, uint16_t pin) : GPIODevice(name),
                                                                           taskLock(1),
                                                                           halGPIO(halGPIO),
                                                                           currentBlocked(-1),
                                                                           pin(pin) {};

    RetType init() override {
        return HALHandlers::register_gpio(halGPIO, this);
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

    RetType set(uint32_t val) override {
        RESUME();

        RetType ret = CALL(taskLock.acquire());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        taskLock = sched_dispatched;

        if (!(val == 0 || val == 1)) {
            RESET();

            return RET_ERROR;
        }

        HAL_GPIO_WritePin(halGPIO, this->pin, static_cast<GPIO_PinState>(val));
        BLOCK();

        currentBlocked = -1;

        ret = CALL(taskLock.release());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) override {
        RESUME();

        RetType ret = CALL(taskLock.acquire());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        taskLock = sched_dispatched;
        *val = static_cast<int>(HAL_GPIO_ReadPin(halGPIO, pin));

        BLOCK();

        currentBlocked = -1;

        ret = CALL(taskLock.release());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    void callback(int) override {
        if (currentBlocked != -1) WAKE(currentBlocked);
    }

private:
    BlockingSemaphore taskLock;
    GPIO_TypeDef *halGPIO;
    tid_t currentBlocked;
    uint16_t pin;

    static const int REG_NUM = 0;
};

#endif //LAUNCH_CORE_HALGPIODEVICE_H
