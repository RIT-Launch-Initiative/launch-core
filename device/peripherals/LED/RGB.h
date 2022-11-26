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
            VIOLET,
            WHITE,
            BLACK
        } RGB_COLOR_T;

    public:
        RGB(GPIODevice &redPin, GPIODevice &greenPin, GPIODevice &bluePin) : redPin(redPin), greenPin(greenPin),
                                                                             bluePin(bluePin) {}

        RGB(GPIODevice &redPin, GPIODevice &greenPin, GPIODevice &bluePin, uint8_t redVal,
            uint8_t greenVal, uint8_t blueVal) :
                redPin(redPin), greenPin(greenPin), bluePin(bluePin), redVal(redVal), greenVal(greenVal),
                blueVal(blueVal) {}

        RetType toggle() {
            RESUME();
            uint32_t result = 0;

            RetType ret = CALL(redPin.get(&result));
            if (ret != RET_SUCCESS) return ret;
            if (result != 0) return turnOff();

            ret = CALL(greenPin.get(&result));
            if (ret != RET_SUCCESS) return ret;
            if (result != 0) return turnOff();

            ret = CALL(bluePin.get(&result));
            if (ret != RET_SUCCESS) return ret;
            if (result != 0) return turnOff();

            ret = CALL(updateColors());
            if (ret != RET_SUCCESS) return ret;

            RESET();
            return ret;
        }

        RetType setColor(uint8_t red, uint8_t green, uint8_t blue) {
            RESUME();

            this->redVal = red;
            this->greenVal = green;
            this->blueVal = blue;

            RetType ret = CALL(updateColors());
            if (ret != RET_SUCCESS) return ret;

            RESET();
            return ret;
        }

        RetType setColor(RGB_COLOR_T color) {
            RESUME();

            switch (color) {
                case RED:
                    return setColor(255, 0, 0);
                case ORANGE:
                    return setColor(255, 127, 0);
                case YELLOW:
                    return setColor(255, 255, 0);
                case GREEN:
                    return setColor(0, 255, 0);
                case BLUE:
                    return setColor(0, 0, 255);
                case INDIGO:
                    return setColor(75, 0, 130);
                case VIOLET:
                    return setColor(148, 0, 211);
                case BLACK:
                    return setColor(0, 0, 0);
                default:
                    return setColor(255, 255, 255);
            }
        }


    private:
        GPIODevice &redPin;
        GPIODevice &greenPin;
        GPIODevice &bluePin;

        // TODO: Worth storing this just for an LED?
        uint8_t redVal = 255;
        uint8_t greenVal = 255;
        uint8_t blueVal = 255;

        RetType turnOff() {
            RESUME();

            CALL(redPin.set(0));
            CALL(bluePin.set(0));
            CALL(greenPin.set(0));

            RESET();
            return RET_SUCCESS;
        }


        RetType updateColors() {
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
