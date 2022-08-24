/// @brief handles HAL callback functions

#ifndef HAL_HANDLERS_H
#define HAL_HANDLERS_H

#include "stm32f4xx_hal.h"

#include "return.h"
#include "device/Device.h"

/// @brief defines a device interface for an aysnchronous callback enabled device
class CallbackDevice : Device {
public:
    /// @brief callback function to be called aysnchronously by an ISR
    /// @param num  some unique number set by the device that registered
    ///             this number is passed back to the device so it can distinguish
    ///             between multiple callbacks if it's registered to multiple
    virtual void callback(int num) = 0;
};

namespace HALHandlers {

/// @brief register a device for UART transmit complete callback
/// @param huart    the UART device to register the callback for
/// @param dev      the device registering
/// @param num      some unique number that will be passed back in the 'callback'
///                 function of 'dev' when the event occurs
/// @return
RetType register_uart_tx(UART_HandleTypeDef* huart, CallbackDevice* dev, int num);

/// @brief register a device for UART receive complete callback
/// @param huart    the UART device to register the callback for
/// @param dev      the device registering
/// @param num      some unique number that will be passed back in the 'callback'
///                 function of 'dev' when the event occurs
/// @return
RetType register_uart_rx(UART_HandleTypeDef* huart, CallbackDevice* dev, int num);

}

#endif
