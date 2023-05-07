/*******************************************************************************
*
*  Name: Device.h
*
*  Purpose: Contains interface for a generic device.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "queue/allocated_queue.h"
#include "return.h"

/// @brief generic device
class Device {
public:
    /// @brief constructor
    Device(const char* name) :  m_uid(uid_counter), m_name(name) {
        uid_counter++;
    };

    /// @brief destructor
    virtual ~Device() {};

    /// @brief initialize the device
    /// @return
    virtual RetType init() = 0;

    /// @brief destroy the device

    /// @brief obtain the device
    /// @return
    virtual RetType obtain() = 0;

    /// @brief release the device
    /// @return
    virtual RetType release() = 0;

    /// @brief poll the device
    ///        any synchronous work for the device should be done here
    ///
    ///        It can also be blocked and woken just like any task function.
    ///        This can be used to block and wait for an ISR as a method to
    ///        schedule interrupt handling.
    ///
    /// @return
    virtual RetType poll() = 0;

    /// @brief get unique ID of this device
    /// @return the unique id
    uint16_t uid() {
        return m_uid;
    }

#ifdef DEBUG
    /// @brief print a textual representation of the device using 'printf'
    virtual void print() {
        printf("%s\t---\tunique ID 0x%04x\r\n", m_name, m_uid);
    }
#endif

protected:
    uint16_t m_uid;
    const char* m_name;

private:
    static uint16_t uid_counter; // should initialize to 0
};

#endif
