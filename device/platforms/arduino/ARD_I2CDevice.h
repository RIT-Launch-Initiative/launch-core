/**
 * Arduino implementation for I2C Device controllers
 *
 * @author Chloe Clark
 */

#ifndef ARD_I2C_DEVICE_H
#define ARD_I2C_DEVICE_H

#include "device/platforms/arduino/Wire/Wire.h"		// TODO fix this later so it can actually find arduino libraries
#include "device/I2CDevice.h"
#include "device/platforms/arduino/ARD_Handlers.h"
#include "sched/macros.h"
#include "sync/BlockingSemaphore.h"

/// @brief I2C device controller
class ARDI2CDevice : public I2CDevice, public CallbackDevice {
public:
    /// @brief constructor
    /// @param name    the name of this device
    /// @param ai2c    the ARD I2C device wrapped by this device
    ARDI2CDevice(const char *name, I2C_HandleTypeDef *ai2c) : 
	    I2CDevice(name),
	    m_blocked(-1),
	    m_i2c(ai2c),
	    m_lock(1),
	    m_isr_flag(0) {};

    /// @brief initialize
    RetType init() {
	    // register for tx callback
	    RetType ret = ARDHandlers::register_i2c_tx(m_i2c, this, TX_NUM);
	    if (ret != RET_SUCCESS) {
	        return ret;
	    }

	    //register for rx callback
	    ret = ARDHandlers::register_i2c_rx(m_i2c, this, RX_NUM);
	    if (ret != RET_SUCCESS) {
	        return ret;
	    }

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
    RetType poll() {
	// disable interrupts to protect access to 'm_isr_flag'
	    __disable_irq();

	    if (m_isr_flag) {
	        // an interrupt occured
	        // reset the flag
	        m_isr_flag = 0;

	        // re-enable interrupts
	        __enable_irq();

	        // if a task was blocked waiting for completion of this ISR, wake it up
	        if (m_blocked != -1) {
		        WAKE(m_blocked);
		        m_blocked = -1;
	        }
	    } else {
	        __enable_irq();
	    }

	    return RET_SUCCESS;
    }

    /// @brief transmit to an I2C device
    /// @param addr     the I2C address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType transmit(I2CAddr_t &addr, uint8_t *buff, size_t len, uint32_t timeout) {
        RESUME();

        RetType ret = CALL(m_lock.acquire()); // Will this work or should I use Wire lib here
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

	m_blocked = sched_dispatched;

	Wire.begin();
	Wire.beginTransmission(addr.dev_addr); // Write to a device number, doubt this will work, seems to want an integer in documentation
	Wire.write(&buff);	// Inherently blocking? Should wait until the buffer gets written and then continue
	Wire.endTransmission;

	ret = CALL(m_lock.release());
	if (ret != RET_SUCCESS) {
	    RESET();
	    return ret;
	}

	RESET();
	return RET_SUCCESS;
    }

	/// @brief transmit to an I2C device
    /// @param addr     the I2C address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
	RetType write(I2CAddr_t &addr, uint8_t *buff, size_t len, uint32_t timeout) {
		RESUME();

		RetType ret = CALL(m_lock.acquire());
        if (ret != RET_SUCCESS) {
            // some error
            RESET();
            return ret;
        }

		m_blocked = sched_dispatched;

		Wire.begin();
		
	}

	/// @brief write to the I2C register
    /// @param addr     the I2C address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
	RetType receive(I2CAddr_t &addr, uint8_t *buff, size_t len, uint32_t timeout) {
	}
	
	/// @brief read from the I2C register
    ///        blocks until enough data is ready
    /// @param addr     the I2C address to read from
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
	RetType read(I2CAddr_t &addr, uint8_t *buff, size_t len, uint32_t timeout) {	// Do I need the timeout? Arduino seems to automatically block
		RESUME();

		// block and wait for the device to be available
        RetType ret = CALL(m_lock.acquire());
        if (ret != RET_SUCCESS) {
            // some error
            RESET();
            return ret;
        }

		m_blocked = sched_dispatched;

		Wire.beginTransmission(addr.dev_addr);
		Wire.write(addr.mem_addr);				
		Wire.requestFrom(addr, len);
		int i = 0;
		while (Wire.available() && i < len) {
			*(buff + i++) = Wire.read();
		} 
		Wire.endTransmission();

		ret = CALL(m_lock.release());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

		RESET();
	}

	/// @brief Request a transfer from the I2C slave
    /// @param addr     the I2C address to read from
    /// @param buff     the buffer to send command and store data in
    /// @param inLen    the number of bytes to send
    /// @param outLen   the number of bytes to receive
    /// @return
    RetType transmitReceive(I2CAddr_t &addr, uint8_t *buff, size_t inLen, size_t outLen, uint32_t timeout, uint8_t secondAddr) {
	}

	/// @brief called by I2C handler asynchronously
    void callback(int) {
        // all this does is set a flag
        // the interrupt is actually "handled" in 'poll'
        m_isr_flag = 1;
    }

	private:
    // unique numbers for tx vs. rx callback
    static const int TX_NUM = 0;
    static const int RX_NUM = 1;

    // currently blocked task
    tid_t m_blocked;

    // HAL I2C handle
    I2C_HandleTypeDef *m_i2c;

    // Device lock
    BlockingSemaphore m_lock;

    // Flag when an interrupt has occurred
    uint8_t m_isr_flag;

};

#endif