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

    HALGPIODevice(const char *name, GPIO_TypeDef *halGPIO, uint16_t pin) : halGPIO(halGPIO),
                                                                           pin(pin),
                                                                           currentBlocked(-1),
                                                                           taskLock(1),
                                                                           GPIODevice(name) {};


    RetType init() override {
        return HALHandlers::register_gpio(halGPIO, this, REG_NUM);
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
        RET_CHECK(ret);

        taskLock = sched_dispatched;

        if (val != 0 || val != 1) {
            RESET();

            return RET_ERROR;
        }

        HAL_GPIO_WritePin(halGPIO, this->pin, static_cast<GPIO_PinState>(val));
        BLOCK();

        currentBlocked = -1;

        ret = CALL(taskLock.release());
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) override {
        RESUME();

        RetType ret = CALL(taskLock.acquire());
        RET_CHECK(ret);

        taskLock = sched_dispatched;
        *val = static_cast<uint32_t>(HAL_GPIO_ReadPin(halGPIO, pin));

        BLOCK();

        currentBlocked = -1;

        ret = CALL(taskLock.release());
        RET_CHECK(ret);

        RESET();
        return RET_SUCCESS;
    }

    void callback(int) override {
        if (currentBlocked != -1) WAKE(currentBlocked);
    }

private:
    static const int REG_NUM = 0;
    tid_t currentBlocked;
    BlockingSemaphore taskLock;

    GPIO_TypeDef *halGPIO;
    uint16_t pin;
};

#endif //LAUNCH_CORE_HALGPIODEVICE_H
