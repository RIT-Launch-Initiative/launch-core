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
        RGB(GPIODevice& redPin, GPIODevice& bluePin, GPIODevice& greenPin) : redPin(redPin), bluePin(bluePin), greenPin(greenPin) {}
        RGB(GPIODevice& redPin, GPIODevice& bluePin, GPIODevice& greenPin, uint8_t redVal, uint8_t blueVal, uint8_t greenVal) :
        redPin(redPin), bluePin(bluePin), greenPin(greenPin), redVal(redVal), blueVal(blueVal), greenVal(greenVal) {}





    private:
        GPIODevice& redPin;
        GPIODevice& bluePin;
        GPIODevice& greenPin;

        uint8_t redVal = 255;
        uint8_t blueVal = 255;
        uint8_t greenVal = 255;

    };
}


#endif //LAUNCH_CORE_RGB_H
