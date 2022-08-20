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

    Pin pin;
    Direction direction;
    Pull pull;

    gpio(Pin pin, Direction direction, Pull pull = Pull::DOWN) : pin(pin), direction(direction), pull(pull) {}

    Pin getPin() const {
        return pin;
    }

    Direction getDirection() const {
        return direction;
    }

    void toggleDirection() {
        this->direction = direction == Direction::OUTPUT ? Direction::INPUT : Direction::OUTPUT;
    }

    Pull getPull() const {
        return pull;
    }

    void setPull(Pull pull) {
        gpio::pull = pull;
    }


};


#endif //LAUNCH_CORE_GPIO_H
