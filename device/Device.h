#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

#include "queue/allocated_queue.h"
#include "return.h"

/// @brief generic device
class Device {
public:
    /// @brief constructor
    Device() : m_uid(uid_counter) {
        uid_counter++;
    };

    /// @brief destructor
    virtual ~Device() {};

    /// @brief initialize the device
    /// @return
    virtual RetType init() = 0;

    /// @brief obtain the device
    /// @return
    virtual RetType obtain() = 0;

    /// @brief release the device
    /// @return
    virtual RetType release() = 0;

    /// @brief poll the device
    ///        any synchronous work for the device should be done here
    /// @return
    virtual RetType poll() = 0;

    /// @brief get unique ID of this device
    /// @return the unique id
    uint16_t uid() {
        return m_uid;
    }

    #ifdef DEBUG
    /// @brief print a textual representation of the device using 'printf'
    virtual void print() = 0;
    #endif

protected:
    uint16_t m_uid;

private:
    static uint16_t uid_counter; // should initialize to 0
};

#endif
