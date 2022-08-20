//
// Created by aaron on 8/19/22.
//

#ifndef LAUNCH_CORE_GPIO_H
#define LAUNCH_CORE_GPIO_H


class gpio {
public:
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




};


#endif //LAUNCH_CORE_GPIO_H
