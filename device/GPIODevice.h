/*******************************************************************************
*
*  Name: GPIODevice.h
*
*  Purpose: Contains the interface for a GPIO device.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef GPIO_DEVICE_H
#define GPIO_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "device/Device.h"
#include "return.h"

/// @brief GPIO device, owns a single pin
class GPIODevice : public Device {
public:
    /// @brief constructor
    GPIODevice(const char* name) : Device(name) {};

    /// @brief set the pin
    ///        this depends on what kind of GPIO it is
    ///        e.g. a digital can only be set to 0 or 1, but a PWM could be 0-255
    ///        we pass in a uint32_t to be safe
    ///        on input pins this may not work
    /// @param val      the value to set the pin to
    /// @return
    virtual RetType set(uint32_t val) = 0;

    /// @brief get the current value of the pin
    ///        value depends on what kind of pin it is
    ///        on output pins this may not work
    /// @param val      where to store the current value of the pin
    virtual RetType get(uint32_t* val) = 0;
};

#endif