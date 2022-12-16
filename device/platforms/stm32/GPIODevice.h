//
// Created by aaron on 12/16/22.
//

#ifndef LAUNCH_CORE_GPIODEVICE_H
#define LAUNCH_CORE_GPIODEVICE_H

#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_def.h"


#include "device/GPIODevice.h"
#include "HAL_Handlers.h"
#include "sched/sched.h"
#include "sync/BlockingSemaphore.h"

class HALGPIODevice : public GPIODevice, public CallbackDevice {
public:

    HALGPIODevice(const char *name, GPIO_HandleTypeDef *halGPIO) : halGPIO(halGPIO),
                                                                   currentBlocked(-1),
                                                                   taskLock(1),
                                                                   GPIODevice(name) {}


    RetType init() {
        return HALHandlers::register_gpio(halGPIO, this, REG_NUM);
    }

    RetType obtain() {
        return RET_SUCCESS;
    }

    RetType poll() {
        return RET_SUCCESS;
    }

    RetType release() {
        return RET_SUCCESS;
    }

    RetType set(uint32_t val) {
        // TODO: Call HAL fns

        return RET_SUCCESS;
    }

    RetType get(uint32_t* val) {
        // TODO Call HAL fn

        return RET_SUCCESS;
    }



private:
    static const int REG_NUM = 0;
    tid_t currentBlocked;
    alloc::Queue<tid_t, MAX_NUM_TASKS> taskQueue;
    BlockingSemaphore taskLock;

    GPIO_HandleTypeDef *halGPIO;
};

#endif //LAUNCH_CORE_GPIODEVICE_H
