//
// Created by aaron on 8/19/22.
//

#ifndef LAUNCH_CORE_GPIO_H
#define LAUNCH_CORE_GPIO_H


class gpio {
public:
    enum class Pin;
    enum class State {
        LOW,
        HIGH
    };

    enum class Direction {
        INPUT,
        OUTPUT
    };

    enum class Pull {
        NONE,
        UP,
        DOWN
    };

    enum class TriggerState {
        RISING,
        FALLING
    };

    Pin pin;
    Direction direction;
    Pull pull;

    gpio(Pin pin, Direction direction, Pull pull = Pull::DOWN) {
        this->pin = pin;
        this->direction = direction;
        this->pull = pull;
    }

    Pin getPin() const {
        return pin;
    }

    void setPin(Pin pin) {
        gpio::pin = pin;
    }

    Direction getDirection() const {
        return direction;
    }

    void setDirection(Direction direction) {
        gpio::direction = direction;
    }

    Pull getPull() const {
        return pull;
    }

    void setPull(Pull pull) {
        gpio::pull = pull;
    }


};


#endif //LAUNCH_CORE_GPIO_H
