/**
 * Device responsible for time related operations
 *
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_TIMERDEVICE_H
#define LAUNCH_CORE_TIMERDEVICE_H

#include "Device.h"

class TimerDevice : public Device {
public:
    TimerDevice(const char* name) : Device(name) {}

    /**
     * Delays execution for a specified period of time
     * @param millis the amount of milliseconds to delay execution
     * @return
     */
    virtual RetType delay(uint32_t millis) = 0;

    /**
     * Get the amount of ticks since start of execution
     * @param tick Stores the current tick number
     * @return
     */
    virtual RetType getTick(uint64_t* tick) = 0;

};
#endif //LAUNCH_CORE_TIMERDEVICE_H
