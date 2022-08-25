#ifndef HAL_SPI_HANDLERS_H
#define HAL_SPI_HANDLERS_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

#include "device/stm32/HAL_handlers.h"

namespace HALHandlers {

// hashmap that maps SPI devices to registered devices
// uses the default XOR hash
static alloc::Hashmap<SPI_HandleTypeDef*, dev_t, MAX_SPI_DEVICES, MAX_SPI_DEVICES> spi_tx_map;
static alloc::Hashmap<SPI_HandleTypeDef*, dev_t, MAX_SPI_DEVICES, MAX_SPI_DEVICES> spi_rx_map;

RetType register_spi_tx(SPI_HandleTypeDef* hspi, CallbackDevice* dev, int num) {
    if(spi_tx_map[hspi] != NULL) {
        // someone is already registered for this device
        // delete them and add us instead
        if(!spi_tx_map.rm(hspi)) {
            return RET_ERROR;
        }
    }

    dev_t* ptr = spi_tx_map.add(h12c);
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
        if(!spi_rx_map.rm(hspi)) {
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

#endif
