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
            RetType ret = RET_ERROR;

            switch (color) {
                case RED: {
                    ret = setColor(255, 0, 0);
                    break;
                }
                case ORANGE: {
                    ret = setColor(255, 127, 0);
                    break;
                }
                case YELLOW: {
                    ret = setColor(255, 255, 0);
                    break;
                }
                case GREEN: {
                    ret = setColor(0, 255, 0);
                    break;
                }
                case BLUE: {
                    ret = setColor(0, 0, 255);
                    break;
                }
                case INDIGO: {
                    ret = setColor(75, 0, 130);
                    break;
                }
                case VIOLET: {
                    ret = setColor(148, 0, 211);
                    break;
                }
                case BLACK: {
                    ret = setColor(0, 0, 0);
                    break;
                }
                default: {
                    ret = setColor(255, 255, 255);
                    break;
                }
            }

            if (ret != RET_SUCCESS) return RET_ERROR;

            RESET();
            return ret;
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

            RetType ret = CALL(redPin.set(0));
            if (ret != RET_SUCCESS) return ret;

            ret = CALL(bluePin.set(0));
            if (ret != RET_SUCCESS) return ret;

            ret = CALL(greenPin.set(0));
            if (ret != RET_SUCCESS) return ret;


            RESET();
            return ret;
        }


        RetType updateColors() {
            RESUME();

            RetType ret = CALL(redPin.set(redVal));
            if (ret != RET_SUCCESS) return ret;

            ret = CALL(bluePin.set(blueVal));
            if (ret != RET_SUCCESS) return ret;

            ret = CALL(greenPin.set(greenVal));
            if (ret != RET_SUCCESS) return ret;


            RESET();
            return ret;
        }

    };
}


#endif //LAUNCH_CORE_RGB_H
