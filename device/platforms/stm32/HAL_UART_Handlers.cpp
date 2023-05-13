#include <stdlib.h>

#ifdef STM32F446xx
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#elif STM32L476xx
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_uart.h"
#endif

#include "device/platforms/stm32/HAL_Handlers.h"
#include "hashmap/hashmap.h"

namespace HALHandlers {

// hashmap that maps UART devices to registered devices
// uses the default XOR hash
static alloc::Hashmap<UART_HandleTypeDef*, dev_t, MAX_UART_DEVICES, MAX_UART_DEVICES> uart_tx_map;
static alloc::Hashmap<UART_HandleTypeDef*, dev_t, MAX_UART_DEVICES, MAX_UART_DEVICES> uart_rx_map;

RetType register_uart_tx(UART_HandleTypeDef* huart, CallbackDevice* dev, int num) {
    if(uart_tx_map[huart] != NULL) {
        // someone is already registered for this device
        // delete them and add us instead
        if(!uart_tx_map.remove(huart)) {
            return RET_ERROR;
        }
    }

    dev_t* ptr = uart_tx_map.add(huart);
    if(ptr == NULL) {
        // failed to add :(
        return RET_ERROR;
    }

    // register this device
    *ptr = {dev, num};

    return RET_SUCCESS;
}

RetType register_uart_rx(UART_HandleTypeDef* huart, CallbackDevice* dev, int num) {
    if(uart_rx_map[huart] != NULL) {
        // someone is already registered for this device
        // delete them and add us instead
        if(!uart_rx_map.remove(huart)) {
            return RET_ERROR;
        }
    }

    dev_t* ptr = uart_rx_map.add(huart);
    if(ptr == NULL) {
        // failed to add :(
        return RET_ERROR;
    }

    // register this device
    *ptr = {dev, num};

    return RET_SUCCESS;
}

} // namspace HALHandlers


// the actual HAL UART callback functions

// transmit complete
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    // lookup if there's a device registered for this UART
    HALHandlers::dev_t* dev = HALHandlers::uart_tx_map[huart];

    // if there's a device, call it's callback function
    if(dev) {
        dev->dev->callback(dev->num);
    }
}

// receive complete
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // lookup if there's a device registered for this UART
    HALHandlers::dev_t* dev = HALHandlers::uart_rx_map[huart];

    // if there's a device, call it's callback function
    if(dev) {
        dev->dev->callback(dev->num);
    }
}

