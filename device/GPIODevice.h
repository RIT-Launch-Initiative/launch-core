//
// Created by Aaron Chan on 8/31/22.
//

#ifndef LAUNCH_CORE_GPIODEVICE_H
#define LAUNCH_CORE_GPIODEVICE_H

#include "Device.h"

class GPIODevice : public Device {
public:
    /**
     * Address of the pin which is hardware specific
     */
    enum class Pin;

    /**
     * State of the GPIO Pin
     */
    enum class State {
        LOW,
        HIGH
    };

    /**
     * State of GPIO Flow
     */
    enum class Direction {
        INPUT,
        OUTPUT
    };

    /**
     * Direction the internal resistor should be in
     */
    enum class Pull {
        NONE,
        UP,
        DOWN
    };

    Pin pin;
    Direction direction;
    Pull pull;


    /**
     * Create a new GPIO instance with a specific pin, direction and pull
     * @param pin
     * @param direction
     * @param pull
     */
    GPIODevice(Pin pin, Direction direction, Pull pull = Pull::DOWN) : pin(pin), direction(direction), pull(pull) {}

    /**
     * Getter for the pin field
     * @return Pin of the GPIO Instance
     */
    Pin getPin() const {
        return pin;
    }

    /**
     * Getter for the pin field
     * @return Direction of the GPIO Pin
     */
    Direction getDirection() const {
        return direction;
    }

    void toggleDirection() {
        this->direction = direction == Direction::OUTPUT ? Direction::INPUT : Direction::OUTPUT;
    }

    /**
     * Getter for the pull field
     * @return Pull of the GPIO Pin
     */
    Pull getPull() const {
        return pull;
    }

    /**
     * Setter for the pull field
     * @param pull
     */
    void setPull(Pull pull) {
        GPIODevice::pull = pull;
    }

    /**
     * Get the state of the pin
     * @return Pin State
     */
    virtual State getState() = 0;
};


#endif //LAUNCH_CORE_GPIODEVICE_H
