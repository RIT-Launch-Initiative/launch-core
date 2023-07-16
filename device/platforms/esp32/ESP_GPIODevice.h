/**
 * ESP32 GPIO Abstraction
 * API Reference: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_ESP_GPIODEVICE_H
#define LAUNCH_CORE_ESP_GPIODEVICE_H

#include <stdint.h>

#include "return.h"

class ESP_GPIODevice : public GPIODevice {
public:
    ESP_GPIODevice(const char *name, gpio_num_t gpio_num) : GPIODevice(name), m_gpio_num(gpio_num) {}

    RetType init() override {
        return RET_SUCCESS;
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        return RET_SUCCESS;
    }

    RetType set(uint32_t val) override {
        if (ESP_ERR_INVALID_ARG == gpio_set_level(m_gpio_num, val)) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) override {
        *val = gpio_get_level(m_gpio_num);

        return RET_SUCCESS;
    }

private:
    gpio_num_t m_gpio_num;
};


#endif //LAUNCH_CORE_ESP_GPIODEVICE_H
