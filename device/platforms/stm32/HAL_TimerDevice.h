/**
 * HAL Implementation of a Timer Device
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_HAL_TIMERDEVICE_H
#define LAUNCH_CORE_HAL_TIMERDEVICE_H

#include "device/TimerDevice.h"
#include "stm32f4xx_hal.h"



class HALTimerDevice : public TimerDevice {
public:
    HALTimerDevice() : TimerDevice("HAL Timer Device") {}

    /**
     * Delays execution for a specified period of time
     * @param millis the amount of milliseconds to delay execution
     * @return
     */
    RetType delay(uint32_t millis) override {

        HAL_Delay(millis);

        return RET_SUCCESS;
    };

    /**
     * Get the amount of ticks since start of execution
     * @param tick Stores the current tick number
     * @return
     */
    RetType getTick(uint64_t* tick) override {
        *tick = HAL_GetTick();

        return RET_SUCCESS;
    }

    RetType init() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        return RET_SUCCESS;
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }


};

#endif //LAUNCH_CORE_HAL_TIMERDEVICE_H
