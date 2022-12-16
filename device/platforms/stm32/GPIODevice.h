//
// Created by aaron on 12/16/22.
//

#ifndef LAUNCH_CORE_GPIODEVICE_H
#define LAUNCH_CORE_GPIODEVICE_H

#include "stm32f4xx_hal_gpio.h"

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
        RetType ret = HALHandlers::register_gpio(halGPIO, this, REG_NUM);



        return RET_SUCCESS;
    }

private:
    static const int REG_NUM = 0halGPIO;
    tid_t currentBlocked;
    alloc::Queue<tid_t, MAX_NUM_TASKS> taskQueue;
    BlockingSemaphore taskLock;

    GPIO_HandleTypeDef *halGPIO;
};

#endif //LAUNCH_CORE_GPIODEVICE_H
