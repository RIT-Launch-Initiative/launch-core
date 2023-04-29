#ifndef HAL_SPI_HANDLERS_H
#define HAL_SPI_HANDLERS_H

#ifdef STM32F446xx
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#elif STM32L476xx
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_spi.h"
#endif

#include "device/platforms/stm32/HAL_Handlers.h"
#include "hashmap/hashmap.h"

namespace HALHandlers {

// hashmap that maps SPI devices to registered devices
// uses the default XOR hash
static alloc::Hashmap<SPI_HandleTypeDef*, dev_t, MAX_SPI_DEVICES, MAX_SPI_DEVICES> spi_tx_map;
static alloc::Hashmap<SPI_HandleTypeDef*, dev_t, MAX_SPI_DEVICES, MAX_SPI_DEVICES> spi_rx_map;

RetType register_spi_tx(SPI_HandleTypeDef* hspi, CallbackDevice* dev, int num) {
    if(spi_tx_map[hspi] != NULL) {
        // someone is already registered for this device
        // delete them and add us instead
        if(!spi_tx_map.remove(hspi)) {
            return RET_ERROR;
        }
    }

    dev_t* ptr = spi_tx_map.add(hspi);
    if(ptr == NULL) {
        // failed to add :(
        return RET_ERROR;
    }

    // register this device
    *ptr = {dev, num};

    return RET_SUCCESS;
}

RetType register_spi_rx(SPI_HandleTypeDef* hspi, CallbackDevice* dev, int num) {
    if(spi_rx_map[hspi] != NULL) {
        // someone is already registered for this device
        // delete them and add us instead
        if(!spi_rx_map.remove(hspi)) {
            return RET_ERROR;
        }
    }

    dev_t* ptr = spi_rx_map.add(hspi);
    if(ptr == NULL) {
        // failed to add :(
        return RET_ERROR;
    }

    // register this device
    *ptr = {dev, num};

    return RET_SUCCESS;
}

} // namespace HALHandlers


// the actual HAL SPI callback functions

// transmit complete
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    // lookup if there's a device registered for this SPI
    HALHandlers::dev_t* dev = HALHandlers::spi_tx_map[hspi];

    // if there's a device, call it's callback function
    if(dev) {
        dev->dev->callback(dev->num);
    }
}

// receive complete
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    // lookup if there's a device registered for this SPI
    HALHandlers::dev_t* dev = HALHandlers::spi_rx_map[hspi];

    // if there's a device, call it's callback function
    if(dev) {
        dev->dev->callback(dev->num);
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    // lookup if there's a device registered for this SPI
    HALHandlers::dev_t* dev = HALHandlers::spi_tx_map[hspi];

    // if there's a device, call it's callback function
    if(dev) {
        dev->dev->callback(dev->num);
    }

    // lookup if there's a device registered for this SPI
    dev = HALHandlers::spi_rx_map[hspi];

    // if there's a device, call it's callback function
    if(dev) {
        dev->dev->callback(dev->num);
    }
}

#endif
