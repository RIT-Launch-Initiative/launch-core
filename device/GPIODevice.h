#ifndef LAUNCH_CORE_GPIODEVICE_H
#define LAUNCH_CORE_GPIODEVICE_H

/**
 * GPIO Device
 *
 * @author Aaron Chan
 */


#include "Device.h"

class GPIODevice : public Device {
public:
    /**
     * State of the GPIO Pin
     */
    enum class State {
        LOW,
        HIGH
    };

    uint8_t pull;
    State state;

    /**
     * Create a new GPIO instance with a specific pin, direction and pull
     * @param pin
     * @param direction
     * @param pull
     */
    GPIODevice(uint8_t pull) : pull(pull) {}

    /**
     * Getter for the pull field
     * @return Pull of the GPIO Pin
     */
    uint8_t getPull() const {
        return pull;
    }

    /**
     * Setter for the pull field
     * @param pull
     */
    void setPull(uint8_t pull) {
        GPIODevice::pull = pull;
    }
};


#endif //LAUNCH_CORE_GPIODEVICE_H
