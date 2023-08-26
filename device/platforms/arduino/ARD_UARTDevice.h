/**
 * Arduino implementation for UART Device controllers
 *
 * @author Brian Tokumoto
 */

#ifndef ARD_UART_DEVICE_H
#define ARD_UART_DEVICE_H

#include <Wire.h>
#include "sched/macros.h"
#include "sync/BlockingSemaphore.h"
#include "return.h"

/// @brief HAL UART device
class ARDUARTDevice :  public StreamDevice {
public:
    /// @brief constructor
    /// @param name    the name of this device
    ARDUARTDevice(const char *name) : StreamDevice(name) {}

    /// @brief initialize
	/// @return always successful
     RetType init() {
        return RET_SUCCESS;
    }

    /// @brief obtain this device
    /// @return always succeeds
    RetType obtain() {
        return RET_SUCCESS;
    }

    /// @brief release this device
    /// @return always succeeds
    RetType release() {
        return RET_SUCCESS;
    }

    /// @brief poll this device
	/// @return always successful
    RetType poll() {
	    return RET_SUCCESS;
    }

    /// @brief write to the UART
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType write(uint8_t *buff, size_t len) {
        
    }

}

#endif