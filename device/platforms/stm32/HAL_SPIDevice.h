#ifndef HAL_SPI_DEVICE_H
#define HAL_SPI_DEVICE_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_gpio.h"

#include "device/SPIDevice.h"
#include "device/platforms/stm32/HAL_Handlers.h"
#include "sched/sched.h"
#include "sync/BlockingSemaphore.h"
#include "device/SPIDevice.h"


/// @brief SPI device controller
class HALSPIDevice : public SPIDevice, public CallbackDevice {
public:
    /// @brief constructor
    /// @param name     the name of this device
    /// @param h12c     the HAL SPI device wrapped by this device
    HALSPIDevice(const char *name, SPI_HandleTypeDef *hspi) : SPIDevice(name),
                                                              m_spi(hspi),
                                                              m_blocked(-1),
                                                              m_lock(1) {};

    /// @brief initialize
    RetType init() {
        // register for tx callback
        RetType ret = HALHandlers::register_spi_tx(m_spi, this, TX_NUM);
        if (ret != RET_SUCCESS) {
            return ret;
        }

        // register for rx callback
        ret = HALHandlers::register_spi_rx(m_spi, this, RX_NUM);
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
    /// @return always successful
    RetType release() {
        return RET_SUCCESS;
    }

    /// @brief poll this device
    /// @return
    RetType poll() {
        // for now does nothing
        return RET_SUCCESS;
    }

    /// @brief write to the SPI
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType write(uint8_t *buff, size_t len) {
        RESUME();

        // block waiting for the device to be free to use
        RetType ret = CALL(m_lock.acquire());
        if (ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // mark this task as blocked BEFORE we call the interrupt function
        // we want to make sure there is no race condition b/w processing the ISR
        // and blocking the task
        m_blocked = sched_dispatched;

        // do our transmit
        if (HAL_OK != HAL_SPI_Transmit_IT(m_spi, buff, len)) {
            return RET_ERROR;
        }

        // block until the transmit is complete
        BLOCK();

        // mark the device as unblocked
        m_blocked = -1;

        // we can unblock someone else if they were waiting
        ret = CALL(m_lock.release());
        if (ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    /// @brief read from the SPI
    ///        blocks until enough data is ready
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
    RetType read(uint8_t *buff, size_t len) {
        RESUME();

        // block waiting for the device to be available
        RetType ret = CALL(m_lock.acquire());
        if (ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // mark this task as blocked BEFORE we call the interrupt function
        // we want to make sure there is no race condition b/w processing the ISR
        // and blocking the task
        m_blocked = sched_dispatched;

        // start the transfer
        if (HAL_OK != HAL_SPI_Receive_IT(m_spi, buff, len)) {
            return RET_ERROR;
        }

        // block waiting for our read to complete
        BLOCK();

        // mark the device as unblocked
        m_blocked = -1;

        // we can unblock someone else if they were waiting
        ret = CALL(m_lock.release());
        if (ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        RESET();
        return ret;
    }

    RetType write_read(uint8_t* write_buff, size_t write_len, uint8_t* read_buff, size_t read_len) {
        RESUME();

        // block waiting for the device to be available
        RetType ret = CALL(m_lock.acquire());
        if (ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // mark this task as blocked BEFORE we call the interrupt function
        // we want to make sure there is no race condition b/w processing the ISR
        // and blocking the task
        m_blocked = sched_dispatched;

        // start the transfer
        if (async) {
            if (HAL_OK != HAL_SPI_TransmitReceive_IT(m_spi, write_buff, read_buff, write_len)) {
                return RET_ERROR;
            }

            // block waiting for our read to complete
            BLOCK();

            // mark the device as unblocked
            m_blocked = -1;
        } else {
            if (HAL_OK != HAL_SPI_TransmitReceive(m_spi, write_buff, read_buff, write_len, 1000)) {
                return RET_ERROR;
            }
        }
        // we can unblock someone else if they were waiting
        ret = CALL(m_lock.release());
        if (ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        RESET();
        return ret;
    }


    void setAsync(bool isAsync) {
        this->async = isAsync;
    }

    /// @brief called by SPI handler asynchronously
    void callback(int) {
        // don't care if it was tx or rx, for now

        if (m_blocked != -1) {
            // wake up the task that's blocked
            WAKE(m_blocked);
        }
    }

private:
    // HAL handle
    SPI_HandleTypeDef *m_spi;

    // current blocked task
    tid_t m_blocked;

    // semaphore
    BlockingSemaphore m_lock;


    // unique numbers for tx vs. rx callback
    static const int TX_NUM = 0;
    static const int RX_NUM = 1;

    bool async;
};

#endif
