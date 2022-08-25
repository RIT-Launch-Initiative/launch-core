#ifndef I2C_DEVICE_H
#define I2C_DEVICE_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"

#include "device/RegisterDevice.h"
#include "device/stm32/HAL_handlers.h"
#include "sched/sched.h"

/// @brief I2C device controller
class I2CDevice : public RegisterDevice<I2CAddr_t>, public CallbackDevice {
public:
    /// @brief constructor
    /// @param name     the name of this device
    /// @param h12c     the HAL I2C device wrapped by this device
    I2CDevice(const char* name, I2C_HandleTypeDef* hi2c) : m_i2c(hi2c),
                                                           m_lock(false),
                                                           m_blocked(-1),
                                                           RegisterDevice<I2CAddr_t>(name) {};

    /// @brief initialize
    RetType init() {
        // register for tx callback
        RetType ret = HALHandlers::register_i2c_tx(m_i2c, this, TX_NUM);
        if(ret != RET_SUCCESS) {
            return ret;
        }

        // register for rx callback
        ret = HALHandlers::register_i2c_rx(m_i2c, this, RX_NUM);
        if(ret != RET_SUCCESS) {
            return ret;
        }

        return RET_SUCCESS;
    }

    /// @brief obtain this device
    ///        for now, the device can only be obtained by a single person
    RetType obtain() {
        if(m_lock) {
            return RET_ERROR;
        }

        m_lock = true;
        return RET_SUCCESS;
    }

    /// @brief release this device
    RetType release() {
        m_lock = false;

        return RET_SUCCESS;
    }

    /// @brief poll this device
    RetType poll() {
        // for now does nothing
        return RET_SUCCESS;
    }

    /// @brief write to the UART
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType write(I2CAddr_t addr, uint8_t* buff, size_t len) {
        RESUME();

        if(HAL_OK != HAL_I2C_Mem_Write_IT(m_i2c, addr.dev_addr, addr.mem_addr,
                                               addr.mem_addr_size, buff, len)) {
            return RET_ERROR;
        }

        m_blocked = sched_dispatched;
        BLOCK();

        RESET();
        return RET_SUCCESS;
    }

    /// @brief read from the UART
    ///        blocks until enough data is ready
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
    RetType read(I2CAddr_t addr, uint8_t* buff, size_t len) {
        RESUME();

        if(HAL_OK != HAL_I2C_Mem_Read_IT(m_i2c, addr.dev_addr, addr.mem_addr,
                                               addr.mem_addr_size, buff, len)) {
            return RET_ERROR;
        }

        m_blocked = sched_dispatched;
        BLOCK();

        RESET();
        return RET_SUCCESS;
    }

private:
    // unique numbers for tx vs. rx callback
    static const int TX_NUM = 0;
    static const int RX_NUM = 1;

    /// @brief called by I2C handler asynchronously
    void callback(int) {
        // don't care if it was tx or rx, for now

        if(m_blocked != -1) {
            WAKE(m_blocked);
        }
    }

    bool m_lock;
    tid_t m_blocked;  // currently blocked task

    I2C_HandleTypeDef* m_i2c;
};

#endif
