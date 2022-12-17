/**
 * Handles GPIO interactions for the STM HAL
 *
 * @author Aaron Chan
 */

#include "hashmap/hashmap.h"
#include "device/platforms/stm32/HAL_Handlers.h"
#include "stm32f446xx.h"

namespace HALHandlers {
    static alloc::Hashmap<GPIO_TypeDef *, CallbackDevice *, MAX_GPIO_DEVICES, MAX_GPIO_DEVICES> gpioMap;

    RetType register_gpio(GPIO_TypeDef *halGPIO, CallbackDevice *callbackDev) {
        if (gpioMap[halGPIO] != nullptr && !gpioMap.remove(halGPIO)) return RET_ERROR;

        CallbackDevice **pCallbackDevice = gpioMap.add(halGPIO);

        if (pCallbackDevice == nullptr) return RET_ERROR;

        *pCallbackDevice = callbackDev;

        return RET_SUCCESS;
    }
}

void HAL_GPIO_CpltCallback(GPIO_TypeDef *halGPIO) {
    CallbackDevice **dev = HALHandlers::gpioMap[halGPIO];
    if (dev) (*dev)->callback(0);
}


