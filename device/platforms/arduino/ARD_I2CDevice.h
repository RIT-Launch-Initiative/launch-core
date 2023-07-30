/**
 * Arduino implementation for I2C Device contrikkers
 *
 * @author Chloe Clark
 */

#ifndef ARD_I2C_DEVICE_H
#define ARD_I2C_DEVICE_H

#include "device/I2CDevice.h"
#include "device/platforms/arduino/ARD_Handlers.h" //TODO make this file
#include "sched/macros.h"
#include "sync/BlockingSemaphore.h"

class ARDI2CDevice : public I2CDevice, public CallbackDevice {
public:
    ARDI2CDevice(const char *name, I2C_HandleTypeDef *hi2c) : 
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


