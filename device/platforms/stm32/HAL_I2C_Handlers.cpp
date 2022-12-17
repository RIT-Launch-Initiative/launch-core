#ifndef HAL_I2C_HANDLERS_H
#define HAL_I2C_HANDLERS_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"

#include "device/platforms/stm32/HAL_Handlers.h"
#include "hashmap/hashmap.h"

namespace HALHandlers {

// hashmap that maps I2C devices to registered devices
// uses the default XOR hash
static alloc::Hashmap<I2C_HandleTypeDef*, dev_t, MAX_I2C_DEVICES, MAX_I2C_DEVICES> i2c_tx_map;
static alloc::Hashmap<I2C_HandleTypeDef*, dev_t, MAX_I2C_DEVICES, MAX_I2C_DEVICES> i2c_rx_map;

RetType register_i2c_tx(I2C_HandleTypeDef* hi2c, CallbackDevice* dev, int num) {
    if(i2c_tx_map[hi2c] != NULL) {
        // someone is already registered for this device
        // delete them and add us instead
        if(!i2c_tx_map.remove(hi2c)) {
            return RET_ERROR;
        }
    }

    dev_t* ptr = i2c_tx_map.add(hi2c);
    if(ptr == NULL) {
        // failed to add :(
        return RET_ERROR;
    }

    // register this device
    *ptr = {dev, num};

    return RET_SUCCESS;
}

RetType register_i2c_rx(I2C_HandleTypeDef* hi2c, CallbackDevice* dev, int num) {
    if(i2c_rx_map[hi2c] != NULL) {
        // someone is already registered for this device
        // delete them and add us instead
        if(!i2c_rx_map.remove(hi2c)) {
            return RET_ERROR;
        }
    }

    dev_t* ptr = i2c_rx_map.add(hi2c);
    if(ptr == NULL) {
        // failed to add :(
        return RET_ERROR;
    }

    // register this device
    *ptr = {dev, num};

    return RET_SUCCESS;
}

} // namespace HALHandlers


// the actual HAL I2C callback functions

// transmit complete
void HAL_I2C_TxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // lookup if there's a device registered for this I2C
    HALHandlers::dev_t* dev = HALHandlers::i2c_tx_map[hi2c];

    // if there's a device, call it's callback function
    if(dev) {
        dev->dev->callback(dev->num);
    }
}

// receive complete
void HAL_I2C_RxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // lookup if there's a device registered for this I2C
    HALHandlers::dev_t* dev = HALHandlers::i2c_rx_map[hi2c];

    // if there's a device, call it's callback function
    if(dev) {
        dev->dev->callback(dev->num);
    }
}

#endif
