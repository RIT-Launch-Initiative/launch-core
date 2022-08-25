#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_gpio.h"

#include "device/StreamDevice.h"
#include "device/stm32/HAL_handlers.h"
#include "sched/sched.h"

// /// @brief defines which GPIO pin and state selects a chip
// typedef struct {
//     GPIO_TypeDef* gpio;     // GPIO device
//     uint16_t pin;           // which pin to change the state of
//     GPIO_PinState state;    // what to set the pin too
// } ChipSelect_t;

// TODO how to be able to read and write before toggling chip select?
// maybe just make the underlying device do this actually

// TODO really need a way to let multiple users use this device
// queue writes and receives? then process them when poll() is called?
// or keep track of multiple TIDs and block a task if someone else is currently reading or writing
// these changes should go for I2CDevice as well

/// @brief SPI device controller
class SPIDevice : public StreamDevice, public CallbackDevice {
public:
    /// @brief constructor
    /// @param name     the name of this device
    /// @param h12c     the HAL SPI device wrapped by this device
    SPIDevice(const char* name, SPI_HandleTypeDef* hspi) :
                                                           m_spi(hspi),
                                                           m_lock(false),
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

    /// @brief write to the SPI
    /// @param buff     the buffer to write
    /// @param len      the size of 'buff' in bytes
    /// @return
    RetType write(uint8_t* buff, size_t len) {
        RESUME();

        if(HAL_OK != HAL_SPI_Transmit_IT(m_spi, buff, len)) {
            return RET_ERROR;
        }

        m_blocked = sched_dispatched;
        BLOCK();

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

        if(HAL_OK != HAL_SPI_Receive_IT(m_spi, buff, len)) {
            return RET_ERROR;
        }

        m_blocked = sched_dispatched;
        BLOCK();

        RESET();
        return RET_SUCCESS;
    }

    /// @brief get the number of available bytes to read
    ///        this will always be 0 for a SPI device,
    ///        all reads must be blocking with the 'read' function
    /// @return the number of available bytes to read, which is always 0
    // TODO this should return 1 if no one is currently blocking on this device, 0 otherwise
    size_t available() {
        return 0;
    }

    /// @brief wait for 'len' bytes to be ready for reading
    ///        for SPI, we only use the blocking 'read' function, so this
    ///        function will always return RET_ERROR
    /// @param len
    /// @return RET_ERROR
    // TODO this should wait until the device is free to be used
    RetType wait(size_t len) {
        return RET_ERROR;
    }

    /// @brief called by SPI handler asynchronously
    void callback(int) {
        // don't care if it was tx or rx, for now

        if(m_blocked != -1) {
            WAKE(m_blocked);
        }
    }

private:
    // unique numbers for tx vs. rx callback
    static const int TX_NUM = 0;
    static const int RX_NUM = 1;

    bool m_lock;
    tid_t m_blocked;  // currently blocked task

    SPI_HandleTypeDef* m_spi;
};

#endif
