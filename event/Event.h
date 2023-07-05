/*******************************************************************************
*
*  Name: event_detection.h
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

    Event(const bool *p_event_detected) : p_event_detected(p_event_detected), num_hooks(0) {}

    /**
     * @brief Task for calculating the event and updating the event detected flag
     *
     * @param args - arguments to pass to the event detection function
     * @return
     */
    virtual RetType calculate_event(void *args);

    /**
     * @brief Get the pointer to the event detected flag
     *
     *
     * @return
     */
    virtual bool *get_event_status() const;

    /**
     * @brief Adds a hook to the event
     *
     * @param hook
     * @return
     */
    constexpr bool add_hook(RetType (*hook)(void *args)) {
        if (num_hooks >= NUM_HOOKS) return false;

        hooks[num_hooks++] = hook;
        return true;
    }

private:
    RetType (*hooks[5])(void *args);

    void *hook_args[5];
    uint8_t num_hooks;

    bool *p_event_detected;

};

#endif //LAUNCH_CORE_EVENT_H
