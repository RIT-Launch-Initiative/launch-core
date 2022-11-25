/**
 * Interface for an LED that has RGB capability
 *
 * @author Aaron Chan
 */

#include "device/peripherals/LED/LED.h"

#ifndef LAUNCH_CORE_RGB_H
#define LAUNCH_CORE_RGB_H

namespace LED {
    class RGB {
    public:
        RGB(GPIODevice& redPin, GPIODevice& bluePin, GPIODevice greenPin) : redPin(redPin), bluePin(bluePin), greenPin(greenPin) {}




    private:
        GPIODevice& redPin;
        GPIODevice& bluePin;
        GPIODevice& greenPin;
    };
}


#endif //LAUNCH_CORE_RGB_H
