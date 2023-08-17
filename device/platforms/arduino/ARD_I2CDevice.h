/**
 * Arduino implementation for I2C Device controllers
 *
 * @author Chloe Clark
 */

#ifndef ARD_I2C_DEVICE_H
#define ARD_I2C_DEVICE_H

#include <Wire.h>		
#include "device/I2CDevice.h"
#include "sched/macros.h"
#include "sync/BlockingSemaphore.h"

/// @brief I2C device controller
class ARDI2CDevice : public I2CDevice{
public:
    /// @brief constructor
    /// @param name    the name of this device
    ARDI2CDevice(const char *name) : 
	    I2CDevice(name),

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

    /// @brief transmit to an I2C device
    /// @param addr     the I2C address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType transmit(I2CAddr_t &addr, uint8_t *buff, size_t len) {

		Wire.begin();
		Wire.beginTransmission(addr.dev_addr); 
		Wire.write(&buff);	
		Wire.endTransmission();

		return RET_SUCCESS;
    }

	/// @brief transmit to an I2C device
    /// @param addr     the I2C address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
	RetType write(I2CAddr_t &addr, uint8_t *buff, size_t len) {

		Wire.begin();
		Wire.beginTransmission(addr.mem_addr);
		Wire.write(&buff);
		Wire.endTransmission();

		return RET_SUCCESS;
	}

	/// @brief read from any I2C device on the wire that transmits
    /// @param addr     the I2C address to read from 
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
	RetType receive(I2CAddr_t &addr, uint8_t *buff, size_t len) {

		Wire.begin();
		int i = 0;
		while (Wire.available() && i < len) {
			*(buff + i++) = Wire.read();
		} 

		return RET_SUCCESS;
	}
	
	/// @brief read from the I2C register
    ///        blocks until enough data is ready
    /// @param addr     the I2C address to read from
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
	RetType read(I2CAddr_t &addr, uint8_t *buff, size_t len) {	

		Wire.begin();
		Wire.beginTransmission(addr.dev_addr);
		Wire.write(addr.mem_addr);				
		Wire.requestFrom(addr, len);
		int i = 0;
		while (Wire.available() && i < len) {
			*(buff + i++) = Wire.read();
		} 
		Wire.endTransmission();

		return RET_SUCCESS;
	}

};

#endif