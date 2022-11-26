/**
 * Interface for an LED
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LED_H
#define LAUNCH_CORE_LED_H

#include "device/GPIODevice.h"
#include "sched/macros.h"

namespace LED {
    typedef enum {
        LED_OFF = 0,
        LED_ON = 1
    } LED_STATE_T;

    class LED {


    public:
        LED(GPIODevice &ledPin) : ledPin(ledPin), ledState(LED_ON) {}

        LED(GPIODevice &ledPin, LED_STATE_T ledState) : ledPin(ledPin), ledState(ledState) {}

        RetType init() {
            RESUME();

            RetType ret = CALL(ledPin.set(ledState));
            if (ret != RET_SUCCESS) return ret;

            RESET();
            return ret;
        }

        RetType toggle() {
            RESUME();

            RetType ret = ledState == LED_OFF ? CALL(setState(LED_ON)) : CALL(setState(LED_OFF));
            if (ret != RET_SUCCESS) return ret;

            RESET();
            return ret;
        }

        RetType setState(LED_STATE_T state) {
            RESUME();

            RetType ret = CALL(ledPin.set(state));
            if (ret != RET_SUCCESS) return ret;

            RESET();
            return ret;
        }

    private:
        GPIODevice &ledPin;
        LED_STATE_T ledState;
    };
}

#endif //LAUNCH_CORE_LED_H
