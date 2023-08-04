/// @brief handles ARD callback functions

#ifndef ARD_HANDLERS_H
#define ARD_HANDLERS_H

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


namespace ARDHandlers {

/// @brief stores information about a registered device
typedef struct {
    CallbackDevice* dev;    // pointer to the registered device
    int num;                // unique number the device registered with
} dev_t;

// maximum number of UART devices supported
static const size_t MAX_UART_DEVICES = 1;  // Looks like atmega chip only supports one uart

// maximum number of I2C devices supported
static const size_t MAX_I2C_DEVICES = 3; 

// maximum number of SPI devices supported
