#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_gpio.h"

#include "device/SPIDevice.h"
#include "device/platforms/stm32/HAL_Handlers.h"
#include "sched/sched.h"

// /// @brief defines which GPIO pin and state selects a chip
// typedef struct {
//     GPIO_TypeDef* gpio;     // GPIO device
//     uint16_t pin;           // which pin to change the state of
//     GPIO_PinState state;    // what to set the pin too
// } ChipSelect_t;

// TODO how to be able to read and write before toggling chip select?
// maybe just make the underlying device do this actually

/// @brief SPI device controller
class HALSPIDevice : public SPIDevice, public CallbackDevice {
public:
    /// @brief constructor
    /// @param name     the name of this device
    /// @param h12c     the HAL SPI device wrapped by this device
    SPIDevice(const char* name, SPI_HandleTypeDef* hspi) :
                                                           m_spi(hspi),
                                                           m_blocked(-1),
                                                           StreamDevice(name) {};

    /// @brief initialize
    RetType init() {
        // register for tx callback
        RetType ret = HALHandlers::register_spi_tx(m_spi, this, TX_NUM);
        if(ret != RET_SUCCESS) {
            return ret;
        }

        // register for rx callback
        ret = HALHandlers::register_spi_rx(m_spi, this, RX_NUM);
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
    /// @return
    RetType poll() {
        // for now does nothing
        return RET_SUCCESS;
    }

    /// @brief write to the SPI
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType write(uint8_t* buff, size_t len) {
        RESUME();

        // block waiting for the device to be free to use
        RetType ret = CALL(check_block());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // mark this task as blocked BEFORE we call the interrupt function
        // we want to make sure there is no race condition b/w processing the ISR
        // and blocking the task
        m_blocked = sched_dispatched;

        // do our transmit
        if(HAL_OK != HAL_SPI_Transmit_IT(m_spi, buff, len)) {
            return RET_ERROR;
        }

        // TODO race condition

        // block until the transmit is complete
        BLOCK();

        // mark the device as unblocked
        m_blocked = -1;

        // we can unblock someone else if they were waiting
        check_unblock();

        RESET();
        return RET_SUCCESS;
    }

    /// @brief read from the SPI
    ///        blocks until enough data is ready
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
    RetType read(uint8_t* buff, size_t len) {
        RESUME();

        // block waiting for the device to be available
        RetType ret = CALL(check_block());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // mark this task as blocked BEFORE we call the interrupt function
        // we want to make sure there is no race condition b/w processing the ISR
        // and blocking the task
        m_blocked = sched_dispatched;

        // start the transfer
        if(HAL_OK != HAL_SPI_Receive_IT(m_spi, buff, len)) {
            return RET_ERROR;
        }

        // block waiting for our read to complete
        BLOCK();

        // mark the device as unblocked
        m_blocked = -1;

        // we can unblock someone else if they were waiting
        check_unblock();

        RESET();
        return ret;
    }

    /// @brief called by SPI handler asynchronously
    void callback(int) {
        // don't care if it was tx or rx, for now

        if(m_blocked != -1) {
            // wake up the task that's blocked
            WAKE(m_blocked);
        }
    }

private:
    // unique numbers for tx vs. rx callback
    static const int TX_NUM = 0;
    static const int RX_NUM = 1;

    // current blocked task
    tid_t m_blocked;

    // queue of tasks waiting on the device to be unblocked
    alloc::Queue<tid_t, MAX_NUM_TASKS> m_queue;

    // HAL handle
    SPI_HandleTypeDef* m_spi;

    /// @brief helper function that blocks the calling task if the device is busy
    /// @return
    RetType check_block() {
        RESUME();

        // someone else is blocked on this device, wait for them
        if(m_blocked != -1) {
            if(!m_queue.push(sched_dispatched)) {
                return RET_ERROR;
            }

            BLOCK();
        } // otherwise we can just return, the device is ours

        RESET();
        return RET_SUCCESS;
    }

    /// @brief helper function that unblocks the next waiting task if there is one
    void check_unblock() {
        tid_t* task = m_queue.peek();

        if(task == NULL) {
            // nothing is waiting
            return;
        }

        // otherwise wake up the next task
        WAKE(*task);
        m_queue.pop();
    }
};

#endif
