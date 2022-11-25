/**
 * Interface for an LED that has RGB capability
 *
 * @author Aaron Chan
 */

#include "device/peripherals/LED/LED.h"

#ifndef LAUNCH_CORE_RGB_H
#define LAUNCH_CORE_RGB_H

namespace LED {
    class RGB : LED {
    public:
        RGB(GPIODevice &ledDevice) : LED(ledDevice), red(255), blue(255), green(255) {}

        RGB(GPIODevice &ledDevice, LED_STATE_T state) : LED(ledDevice, state), red(255), blue(255), green(255) {}

        RGB(GPIODevice &ledDevice, uint8_t red = 0, uint8_t blue = 0, uint8_t green = 0) : LED(ledDevice), red(red),
                                                                                           blue(blue), green(green) {}

        RGB(GPIODevice &ledDevice, LED_STATE_T state, uint8_t red = 0, uint8_t blue = 0, uint8_t green = 0) : LED(ledDevice, state), red(red),
                                                                                      blue(blue), green(green) {}



    private:
        uint8_t red;
        uint8_t blue;
        uint8_t green;
    };
}


#endif //LAUNCH_CORE_RGB_H
