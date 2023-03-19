#ifndef UART_DEVICE_H
#define UART_DEVICE_H

#ifdef STM32F446xx
#include "stm32f4xx_hal_uart.h"
#elif STM32L476xx
#include "stm32l4xx_hal_uart.h"
#endif

#include "device/StreamDevice.h"
#include "device/platforms/stm32/HAL_Handlers.h"
#include "sched/macros.h"
#include "ringbuffer/RingBuffer.h"
#include "sync/BlockingSemaphore.h"

/// @brief HAL UART device
///        essentially wraps HAL_UART functions
class HALUARTDevice : public StreamDevice, public CallbackDevice {
public:
    /// @brief constructor
    /// @param name     the name of this device
    /// @param huart    the HAL UART device wrapped by this device
    HALUARTDevice(const char *name, UART_HandleTypeDef *huart) : StreamDevice(name),
                                                                 m_buff(),
                                                                 m_uart(huart),
                                                                 m_lock(1),
                                                                 m_blocked(-1),
                                                                 m_waiting(false) {};

    /// @brief initialize
    RetType init() {
        // register for tx callback
        RetType ret = HALHandlers::register_uart_tx(m_uart, this, TX_NUM);
        if(ret != RET_SUCCESS) {
            return ret;
        }

        // register for rx callback
        ret = HALHandlers::register_uart_rx(m_uart, this, RX_NUM);
        if(ret != RET_SUCCESS) {
            return ret;
        }

        // start the first read
        if(HAL_OK != HAL_UART_Receive_IT(m_uart, &m_byte, sizeof(uint8_t))) {
            return RET_ERROR;
        }

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
    /// @return
    RetType poll() {
        // for now does nothing
        return RET_SUCCESS;
    }

    /// @brief write to the UART
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType write(uint8_t* buff, size_t len) {
        RESUME();

        // block waiting for the device to be free
        RetType ret = CALL(m_lock.acquire());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // mark this task as blocked BEFORE we call the interrupt function
        // we want to make sure there is no race condition b/w processing the ISR
        // and blocking the task
        m_blocked = sched_dispatched;

        // start the write
        if(HAL_OK != HAL_UART_Transmit_IT(m_uart, const_cast<uint8_t *>(buff), len)) {
            return RET_ERROR;
        }

        // block and wait for our transmit to complete
        BLOCK();

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

    /// @brief read from the UART
    ///        blocks until enough data is ready
    /// @param buff     the buffer to read into
    /// @param len      the number of bytes to read
    /// @return
    RetType read(uint8_t* buff, size_t len) {
        RESUME();

        // block until the device is free to use
        RetType ret = CALL(m_lock.acquire());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // wait until we have enough data
        // we keep the device blocked so no one else can read while we wait
        ret = CALL(wait(len));
        if(ret == RET_SUCCESS) {
            // read the data
            if(len != m_buff.pop(buff, len)) {
                ret = RET_ERROR;
            }
        }

        // we can unblock someone else if they were waiting
        ret = CALL(m_lock.release());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        RESET();
        return ret;
    }

    /// @brief get how many bytes are available for reading without blocking
    /// @return the number of bytes available
    size_t available() {
        return m_buff.size();
    }

    /// @brief wait for a certain amount of data to be ready for reading
    ///        blocks the current process, caller must return to scheduler
    ///        as soon as possible
    /// @return RET_BLOCKED or error
    RetType wait(size_t len) {
        RESUME();

        m_waiting = len;
        m_blocked = sched_dispatched;

        // block the task, even if it might not need to be blocked
        // we want to avoid the case where we check the size and then block, but
        // we get an interrupt in between the check and block, so our task may sleep forever
        sched_block(sched_dispatched);

        // check for one of two cases
        //  1. We already had enough data buffered to read, there was no need to block at all
        //  2. We got an interrupt after setting m_waiting or m_blocked that filled the buffer up, we don't need to block
        // any other case means the buffer is too small and we should block
        // we don't need to worry if we get an interrupt b/w when we check the size
        // and when we yield back to the scheduler, our task is already recorded and
        // the callback handler will wake us (or we may already be woken by the time the function exits)

        if(m_buff.size() >= len) {
            // either case 1 or 2 occured, so we can just wake our task and be done
            sched_wake(sched_dispatched);

            RESET();
            return RET_SUCCESS;
        }

        // otherwise we currently do not have enough data
        // we already blocked our task to avoid race conditions
        // so we just need to give execution back to the scheduler at this point
        YIELD();

        // if we made it here, we got enough data and the callback woke us up
        // oh yeah

        RESET();
        return RET_SUCCESS;
    }

private:
    // TODO not sure what size this should be yet
    alloc::RingBuffer<256, true> m_buff;

    UART_HandleTypeDef *m_uart; // HAL handler

    BlockingSemaphore m_lock; // semaphore

    tid_t m_blocked;  // currently blocked task
    size_t m_waiting; // amount of data the blocked task is waiting for (if rx)

    // single byte to read into
    uint8_t m_byte;


    // unique numbers for tx vs. rx callback
    static const int TX_NUM = 0;
    static const int RX_NUM = 1;

    /// @brief called by UART handler asynchronously
    void callback(int num) {
        if (num == TX_NUM) {
            // transmit complete
            if (m_blocked != -1) {
                // some task blocked on us transmitting
                WAKE(m_blocked);
                m_blocked = -1;
            }
        } else {
            // we received some data into 'm_byte'
            m_buff.push(&m_byte, sizeof(uint8_t)); // don't error check

            // if someone is blocked on reading, see if we can now unblock them
            if (m_blocked == -1) {
                if (m_buff.size() >= m_waiting) {
                    // someone was waiting, wake them up now
                    WAKE(m_blocked);
                }
            }

            // start another read
            HAL_UART_Receive_IT(m_uart, &m_byte, sizeof(uint8_t));
        }

        return;
    }
};

#endif
