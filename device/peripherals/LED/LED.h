/**
 * Interface for an LED
 *
 * @author Aaron Chan, Yevgeniy Gorbachev
 */

#ifndef LAUNCH_CORE_LED_H
#define LAUNCH_CORE_LED_H

#include "device/GPIODevice.h"
#include "sched/macros.h"

typedef enum {
    LED_OFF = 0,
    LED_ON = 1
} LED_STATE_T;

class LED {
public:
    LED(GPIODevice &gpio) : m_gpio(gpio), m_state(LED_ON) {}

    LED(GPIODevice &gpio, LED_STATE_T ledState) : m_gpio(gpio), m_state(ledState) {}

    RetType init() {
        RESUME();
        RetType ret;

        ret = CALL(m_gpio.set(m_state));

        RESET();
        return ret;
    }

    RetType toggle() {
        RESUME();
        RetType ret;

        if (LED_OFF == m_state) {
            m_state = LED_ON;
        } else if (LED_ON == m_state) {
            m_state = LED_OFF;
        }

        ret = CALL(set_state(m_state));

        RESET();
        return ret;
    }

    RetType set_state(LED_STATE_T state) {
        RESUME();
        RetType ret;

        m_state = state;
        ret = CALL(m_gpio.set(m_state));

        RESET();
        return ret;
    }

    RetType set_flash(uint32_t on_time, uint32_t period) {
        if (period < on_time) {
            return RET_ERROR;
        }

        if (0 != on_time) {
            m_on_time = on_time;
        }
        if (0 != period) {
            m_period = period;
        }

        return RET_SUCCESS;
    }

    RetType flash() {
        RESUME();
        RetType ret;

        if (LED_OFF == m_state) {
            ret = CALL(set_state(LED_ON));
            SLEEP(m_on_time);
        } else if (LED_ON == m_state) {
            ret = CALL(set_state(LED_OFF));
            SLEEP(m_period - m_on_time);
        }

        RESET();
        return ret;
    }

private:
    GPIODevice& m_gpio;
    LED_STATE_T m_state;
    uint32_t m_on_time;
    uint32_t m_period;

};

#endif //LAUNCH_CORE_LED_H
