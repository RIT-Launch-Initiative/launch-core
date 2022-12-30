#ifndef HAL_I2C_DEVICE_H
#define HAL_I2C_DEVICE_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"

#include "device/I2CDevice.h"
#include "device/platforms/stm32/HAL_Handlers.h"
#include "sched/macros.h"
#include "sync/BlockingSemaphore.h"

/// @brief I2C device controller
class HALI2CDevice : public I2CDevice, public CallbackDevice {
public:
    /// @brief constructor
    /// @param name     the name of this device
    /// @param h12c     the HAL I2C device wrapped by this device
    HALI2CDevice(const char *name, I2C_HandleTypeDef *hi2c) : I2CDevice(name),
                                                              m_blocked(-1),
                                                              m_i2c(hi2c),
                                                              m_lock(1) {};


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
    /// @return always successful
    RetType obtain() {
        return RET_SUCCESS;
    }

    /// @brief release this device
    /// @return always successful
    RetType release() {
        return RET_SUCCESS;
    }

    /// @brief poll this device
    RetType poll() {
        // for now does nothing
        return RET_SUCCESS;
    }

    /// @brief write to the I2C
    /// @param addr     the I2C address to write to
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType write(I2CAddr_t& addr, uint8_t* buff, size_t len) {
        RESUME();

        // block and wait for the device to be available
        RetType ret = CALL(m_lock.acquire());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // mark this task as blocked BEFORE we call the interrupt function
        // we want to make sure there is no race condition b/w processing the ISR
        // and blocking the task
        m_blocked = sched_dispatched;

        // start the transfer
        if (async){
            if (HAL_OK != HAL_I2C_Mem_Write_IT(m_i2c, addr.dev_addr, addr.mem_addr,
                                               addr.mem_addr_size, buff, len)) {
                return RET_ERROR;
            }
        } else {
            if (HAL_OK != HAL_I2C_Mem_Write(m_i2c, addr.dev_addr, addr.mem_addr,
                                            addr.mem_addr_size, buff, len, 1000)) {
                return RET_ERROR;
            }

            // block and wait for the transfer to complete
            BLOCK();
        }



        // mark the device as unblocked
        m_blocked = -1;

        ret = CALL(m_lock.release());
        if(ret != RET_SUCCESS) {
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    /// @brief read from the I2C
    ///        blocks until enough data is ready
    /// @param addr     the I2C address to read from
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
    RetType read(I2CAddr_t& addr, uint8_t* buff, size_t len) {
        RESUME();

        // block and wait for the device to be available
        RetType ret = CALL(m_lock.acquire());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // mark this task as blocked BEFORE we call the interrupt function
        // we want to make sure there is no race condition b/w processing the ISR
        // and blocking the task
        m_blocked = sched_dispatched;


        // start the transfer
        if (!async){
            if (HAL_OK != HAL_I2C_Mem_Read_IT(m_i2c, addr.dev_addr, addr.mem_addr,
                                              addr.mem_addr_size, buff, len)) {
                return RET_ERROR;
            }
        } else {
            if (HAL_OK != HAL_I2C_Mem_Read(m_i2c, addr.dev_addr, addr.mem_addr,
                                              addr.mem_addr_size, buff, len, 1000)) {
                return RET_ERROR;
            }

            // wait for the transfer to complete
            BLOCK();
        }


        // mark the device as unblocked
        m_blocked = -1;

        // we can unblock someone else if they were waiting
        ret = CALL(m_lock.release());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }


    /// @brief called by I2C handler asynchronously
    void callback(int) {
        // don't care if it was tx or rx, for now

        if(m_blocked != -1) {
            WAKE(m_blocked);
        }
    }

    void setAsync(bool async){
        this->async = async;
    }

private:
    // unique numbers for tx vs. rx callback
    static const int TX_NUM = 0;
    static const int RX_NUM = 1;

    // currently blocked task
    tid_t m_blocked;

    // HAL I2C handle
    I2C_HandleTypeDef* m_i2c;

    // semaphore
    BlockingSemaphore m_lock;

    bool async;
};

#endif
