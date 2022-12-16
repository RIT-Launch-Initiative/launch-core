/**
 * Handles GPIO interactions for the STM HAL
 *
 * @author Aaron Chan
 */

#include "hashmap/hashmap.h"
#include "device/platforms/stm32/HAL_Handlers.h"

namespace HALHandlers {
    static alloc::Hashmap<GPIO_HandleTypeDef *, dev_t, MAX_GPIO_DEVICES, MAX_GPIO_DEVICES> gpioMap;

    RetType register_gpio(GPIO_HandleTypeDef *halGPIO, CallbackDevice *dev, int num) {
        if (gpioMap[halGPIO] != NULL) {
            if (!gpioMap.rm(halGPIO)) return RET_ERROR;
        }

        dev_t *pDev = gpioMap.add(halGPIO);

        if (pDev == NULL) return RET_ERROR;

        *pDev = {dev, num};

        return RET_SUCCESS;
    }
}

void HAL_GPIO_CpltCallback(GPIO_HandleTypeDef *halGPIO) {
    HALHandlers::dev_t *dev = HALHandlers::gpioMap[halGPIO];
    if (dev) dev->dev->callback(dev->num);
}


