/**
 * HAL Implementation for GPIO Devices
 * Supports external interrupts
 *
 * @author Nate Aquino
 */

#ifndef HAL_GPIO_HANDLERS_H
#define HAL_GPIO_HANDLERS_H

#ifdef STM32F446xx
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#elif STM32L476xx
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#endif

#include "HAL_GPIODevice.h"
#include "device/platforms/stm32/HAL_Handlers.h"
#include "hashmap/hashmap.h"

#define MAX_GPIO_DEVICES 4  // idk what to set this to so its 4 for now

namespace HALHandlers {

/// @brief This is a map that maps a gpio port struct pointer to a tuple of a device and a unique number
static alloc::Hashmap<GPIO_TypeDef *, dev_t, MAX_GPIO_DEVICES, MAX_GPIO_DEVICES> gpio_exti_map;

// register a device for GPIO interrupt

/// @brief register a device for GPIO interrupt.
/// This function stores a mapping between the GPIO port and a tuple of the device and a unique number
///     this then allows us to call the callback function of `dev`.
/// @param dev the device registering this callback
/// @param num some unique number that will be passed back in the 'callback' function of `dev` when the event occurs
RetType register_gpio_exti(GPIO_TypeDef *hgpio, CallbackDevice *dev, int num) {
    if (gpio_exti_map[hgpio] != NULL && !gpio_exti_map.remove(hgpio))
        return RET_ERROR;  // error out because a mapping exists and we can't remove it

    // get a pointer to the dev_t device struct in the hashmap
    dev_t *mappingPtr = gpio_exti_map.add(hgpio);

    // check if we failed to add, by seeing if the pointer is null
    if (mappingPtr == NULL)
        return RET_ERROR;  // error out because it failed to add

    // register this device by dereferencing the mapping pointer and setting the values
    *mappingPtr = {dev, num};

    return RET_SUCCESS;  // success 😳👍
}
}  // namespace HALHandlers

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    /// TODO: implement here?
    if (GPIO_Pin == GPIO_PIN_10) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        // HAL_UART_Transmit(&huart2, (uint8_t *)"Triggered\r\n", 11, 1000);
    }
}

#endif  // !HAL_GPIO_HANDLERS_H