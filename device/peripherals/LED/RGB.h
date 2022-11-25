/**
 * Interface for an LED that has RGB capability
 *
 * @author Aaron Chan
 */

#include "device/GPIODevice.h"
#include "sched/macros.h"

#ifndef LAUNCH_CORE_RGB_H
#define LAUNCH_CORE_RGB_H

namespace LED {
    class RGB {
        typedef enum {
            RED,
            ORANGE,
            YELLOW,
            GREEN,
            BLUE,
            INDIGO,
            VIOLET
        } RGB_COLOR_T;

    public:
        RGB(GPIODevice &redPin, GPIODevice &greenPin, GPIODevice &bluePin) : redPin(redPin), greenPin(greenPin),
                                                                             bluePin(bluePin) {}

        RGB(GPIODevice &redPin, GPIODevice &greenPin, GPIODevice &bluePin, uint8_t redVal,
            uint8_t greenVal, uint8_t blueVal) :
                redPin(redPin), greenPin(greenPin), bluePin(bluePin), redVal(redVal), greenVal(greenVal),
                blueVal(blueVal) {}

        RetType toggle() {
            uint32_t result = 0;

            redPin.get(&result);
            if (result != 0) return turnOff();

            bluePin.get(&result);
            if (result != 0) return turnOff();

            greenPin.get(&result);
            if (result != 0) return turnOff();

            return setColors();
        }

        RetType setColor(uint8_t red, uint8_t blue, uint8_t green) {

        }

        RetType setColor(RGB_COLOR_T color) {


        }


    private:
        GPIODevice &redPin;
        GPIODevice &bluePin;
        GPIODevice &greenPin;

        // TODO: Worth storing this just for an LED?
        uint8_t redVal = 255;
        uint8_t blueVal = 255;
        uint8_t greenVal = 255;

        RetType turnOff() {
            RESUME();

            CALL(redPin.set(0));
            CALL(bluePin.set(0));
            CALL(greenPin.set(0));

            RESET();
            return RET_SUCCESS;
        }


        RetType setColors() {
            RESUME();

            CALL(redPin.set(redVal));
            CALL(bluePin.set(blueVal));
            CALL(greenPin.set(greenVal));

            RESET();
            return RET_SUCCESS;
        }

    };
}


#endif //LAUNCH_CORE_RGB_H
