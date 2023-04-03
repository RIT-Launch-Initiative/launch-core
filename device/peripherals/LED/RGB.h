/**
 * Interface for an LED that has RGB capability
 *
 * @author Aaron Chan
 */

#include "device/GPIODevice.h"
#include "sched/macros/macros.h"

#ifndef LAUNCH_CORE_RGB_H
#define LAUNCH_CORE_RGB_H

#define COLOR_SET(r, g, b) {ret = setValues(r, g, b); break;}

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

class RGB {

public:
    RGB(GPIODevice &redPin, GPIODevice &greenPin, GPIODevice &bluePin) : redPin(redPin), greenPin(greenPin),
                                                                         bluePin(bluePin) {}

    RGB(GPIODevice &redPin, GPIODevice &greenPin, GPIODevice &bluePin, uint8_t redVal,
        uint8_t greenVal, uint8_t blueVal) :
            redPin(redPin), greenPin(greenPin), bluePin(bluePin), redVal(redVal), greenVal(greenVal),
            blueVal(blueVal) {}

    RetType init() {
        RESUME();

        RetType ret = CALL(setValues(redVal, greenVal, blueVal));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;

    }

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

    RetType setValues(uint8_t red, uint8_t green, uint8_t blue) {
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
            case RED: COLOR_SET(255, 0, 0)
            case ORANGE: COLOR_SET(255, 0, 0)
            case YELLOW: COLOR_SET(255, 255, 0)
            case GREEN: COLOR_SET(0, 255, 0)
            case BLUE: COLOR_SET(0, 0, 255)
            case INDIGO: COLOR_SET(75, 0, 130)
            case VIOLET: COLOR_SET(148, 0, 211)
            case BLACK: COLOR_SET(0, 0, 0)
            default: COLOR_SET(255, 255, 255)
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

        ret = CALL(greenPin.set(greenVal));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(bluePin.set(blueVal));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }

};


#endif //LAUNCH_CORE_RGB_H
