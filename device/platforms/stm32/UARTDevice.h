#ifndef UART_DEVICE_H
#define UART_DEVICE_H

#include "stm32f4xx_hal_uart.h"

#include "device/StreamDevice.h"
#include "device/platforms/stm32/HAL_Handlers.h"
#include "sched/macros.h"
#include "ringbuffer/RingBuffer.h"

/// @brief HAL UART device
///        essentially wraps HAL_UART functions
class UARTDevice : public StreamDevice, public CallbackDevice {
public:
    /// @brief constructor
    /// @param name     the name of this device
    /// @param huart    the HAL UART device wrapped by this device
    UARTDevice(const char* name, UART_HandleTypeDef* huart) : m_uart(huart),
                                                              m_blocked(-1),
                                                              m_waiting(false),
                                                              m_buff(),
                                                              StreamDevice(name) {};

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
        RetType ret = CALL(check_block());
        if(ret != RET_SUCCESS) {
            // some error
            return ret;
        }

        // start the write
        if(HAL_OK != HAL_UART_Transmit_IT(m_uart, &m_byte, sizeof(uint8_t))) {
            return RET_ERROR;
        }

        // block and wait for our transmit to complete
        m_blocked = sched_dispatched;
        BLOCK();

        // we can unblock someone else if they were waiting
        check_unblock();

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
        RetType ret = CALL(check_block());
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
        check_unblock();

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

        // no need to wait, we already have the data
        if(m_buff.size() >= len) {
            return RET_SUCCESS;
        }

        m_blocked = sched_dispatched;
        m_waiting = len;

        BLOCK();

        RESET();
        return RET_SUCCESS;
    }

private:
    // unique numbers for tx vs. rx callback
    static const int TX_NUM = 0;
    static const int RX_NUM = 1;

    /// @brief called by UART handler asynchronously
    void callback(int num) {
        if(num == TX_NUM) {
            // transmit complete
            if(m_blocked != -1) {
                // some task blocked on us transmitting
                WAKE(m_blocked);
                m_blocked = -1;
            }
        } else {
            // we received some data into 'm_byte'
            m_buff.push(&m_byte, sizeof(uint8_t)); // don't error check

            // if someone is blocked on reading, see if we can now unblock them
            if(m_blocked == -1) {
                if(m_buff.size() >= m_waiting) {
                    // someone was waiting, wake them up now
                    WAKE(m_blocked);
                }
            }

            // start another read
            HAL_UART_Receive_IT(m_uart, &m_byte, sizeof(uint8_t));
        }

        return;
    }

    // TODO not sure what size this should be yet
    alloc::RingBuffer<256, true> m_buff;

    tid_t m_blocked;  // currently blocked task
    size_t m_waiting; // amount of data the blocked task is waiting for (if rx)

    // HAL handler
    UART_HandleTypeDef* m_uart;

    // single byte to read into
    uint8_t m_byte;

    // queue of tasks waiting on the device to be unblocked
    alloc::Queue<tid_t, MAX_NUM_TASKS> m_queue;


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
        }

        BLOCK();

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
