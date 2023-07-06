/*******************************************************************************
*
*  Name: Event.h
*
*  Purpose: Interface for defining event detection functions
*
*  Author: Aaron Chan
*
*  RIT Launch Initiative
*
*******************************************************************************/

#ifndef LAUNCH_CORE_EVENT_H
#define LAUNCH_CORE_EVENT_H

#include <stdint.h>
#include <stdbool.h>
#include "return.h"

class Event {
public:
    const uint8_t NUM_HOOKS = 5;
    const uint8_t DETECT_COUNT = 5;


    Event(const bool *p_event_detected) : p_event_detected(p_event_detected), num_hooks(0) {}

    /**
     * @brief Task for calculating the event and updating the event detected flag
     *
     * @return
     */
    virtual RetType calculate_event() = 0;

    /**
     * @brief Get the pointer to the event detected flag
     *
     *
     * @return
     */
    bool *get_event_status() const {
        return p_event_detected;
    };

    /**
     * @brief Adds a hook to the event
     *
     * @param hook
     * @return
     */
    constexpr bool register_callback(RetType (*hook)(void *args)) {
        if (num_hooks >= NUM_HOOKS) return false;

        hooks[num_hooks++] = hook;
        return true;
    };

private:
    RetType (*hooks[5])(void *args);
    RetType return_values[5] = {RET_SUCCESS, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS};
    void *hook_args[5];

    uint8_t num_hooks;

protected:
    bool *p_event_detected;

    /**
     * @brief Calls all hooks for the event
     */
    void call_hooks() {
        for (uint8_t i = 0; i < num_hooks; i++) {
             sched_start(hooks[i], hook_args[i]);
        }
    }
};

#endif //LAUNCH_CORE_EVENT_H
