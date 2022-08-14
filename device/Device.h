#ifndef DEVICE_H
#define DEVICE_H

#include "return.h"

/// @brief generic device
class Device {
public:
    /// @brief initialize the device
    /// @return
    virtual RetType init() = 0;

    /// @brief obtain the device
    /// @return
    virtual RetType obtain() = 0;

    /// @brief release the device
    /// @return
    virtual RetType release() = 0;
};

#endif
