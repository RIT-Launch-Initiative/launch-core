#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

#include "device/SPIDevice.h"
#include "sync/BlockingSemaphore.h"
#include <SPI.h>

/// @brief SPI controller device
class ARDSPIDevice : public SPIDevice {
public:

    /// @brief constructor
    ARDSPIDevice(const char* name) : SPIDevice(name) {};

    /// @brief initialize
	/// @return always successful
     RetType init() {
        return RET_SUCCESS;
    }

    /// @brief obtain this device
    /// @return always successful
    RetType obtain() {
	    return RET_SUCCESS;
    }

    /// @brief release this device
    /// @brief always successful
    RetType release() {
	    return RET_SUCCESS;
    }

    /// @brief poll this device
	/// @return always successful
    RetType poll() {
	    return RET_SUCCESS;
    }

    /// @brief write to the SPI
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType write(uint8_t *buff, size_t len, uint32_t timeout) {
        SPI.begin();
        SPI.transfer(buff,len);
        SPI.end();
    }

    RetType read(uint8_t *buff, size_t len, uint32_t timeout) {

    }
}
#endif