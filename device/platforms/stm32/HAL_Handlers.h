/// @brief handles HAL callback functions

#ifndef HAL_HANDLERS_H
#define HAL_HANDLERS_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_gpio.h"


#include "return.h"
#include "device/Device.h"

/// @brief defines a device interface for an aysnchronous callback enabled device
class CallbackDevice {
public:
    /// @brief callback function to be called aysnchronously by an ISR
    /// @param num  some unique number set by the device that registered
    ///             this number is passed back to the device so it can distinguish
    ///             between multiple callbacks if it's registered to multiple
    virtual void callback(int num) = 0;
};


namespace HALHandlers {

/// @brief stores information about a registered device
typedef struct {
    CallbackDevice* dev;    // pointer to the registered device
    int num;                // unique number the device registered with
} dev_t;

// maximum number of UART devices supported
static const size_t MAX_UART_DEVICES = 5;

// maximum number of I2C devices supported
static const size_t MAX_I2C_DEVICES = 3;

// maximum number of SPI devices supported
static const size_t MAX_SPI_DEVICES = 3;

// maximum number of GPIO devices supported
static const size_t MAX_GPIO_DEVICES = 5;

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

/// @brief register a device for I2C transmit complete callback
/// @param hi2c     the I2C device to register the callback for
/// @param dev      the device registering
/// @param num      some unique number that will be passed back in the 'callback'
///                 function of 'dev' when the event occurs
/// @return
RetType register_i2c_tx(I2C_HandleTypeDef* hi2c, CallbackDevice* dev, int num);

/// @brief register a device for I2C receive complete callback
/// @param hi2c     the I2C device to register the callback for
/// @param dev      the device registering
/// @param num      some unique number that will be passed back in the 'callback'
///                 function of 'dev' when the event occurs
/// @return
RetType register_i2c_rx(I2C_HandleTypeDef* huart, CallbackDevice* dev, int num);

/// @brief register a device for SPI transmit complete callback
/// @param hspi     the SPI device to register the callback for
/// @param dev      the device registering
/// @param num      some unique number that will be passed back in the 'callback'
///                 function of 'dev' when the event occurs
/// @return
RetType register_spi_tx(SPI_HandleTypeDef* hspi, CallbackDevice* dev, int num);

/// @brief register a device for SPI receive complete callback
/// @param hspi     the SPI device to register the callback for
/// @param dev      the device registering
/// @param num      some unique number that will be passed back in the 'callback'
///                 function of 'dev' when the event occurs
/// @return
RetType register_spi_rx(SPI_HandleTypeDef* hspi, CallbackDevice* dev, int num);

/// @brief register a device for GPIO complete callback
/// @param halGPIO     the GPIO device to register the callback for
/// @param dev      the device registering
/// @param num      some unique number that will be passed back in the 'callback'
///                 function of 'dev' when the event occurs
/// @return
RetType register_gpio(GPIO_TypeDef* halGPIO, CallbackDevice* dev, int num);

}

#endif
