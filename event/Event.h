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
#include "sched/sched.h"


class Event {
public:
    const uint8_t NUM_HOOKS = 5;
    const uint8_t DETECT_COUNT = 5;

    Event(bool* const p_event_detected) : num_callbacks(0), p_event_detected(p_event_detected) {}

    virtual ~Event() = default;

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
    [[nodiscard]] bool const *get_event_status() const {
        return p_event_detected;
    };

    /**
     * @brief Adds a hook to the event
     *
     * @param hook
     * @return
     */
    constexpr bool register_callback(RetType (*hook)(void *args)) {
        if (num_callbacks >= NUM_HOOKS) return false;

        callbacks[num_callbacks++] = hook;
        return true;
    };

private:
    RetType (*callbacks[5])(void *args);
    void *callback_args[5];

    uint8_t num_callbacks;

protected:
    bool *const p_event_detected;

    /**
     * @brief Calls all hooks for the event
     */
    void call_hooks() {
        for (uint8_t i = 0; i < num_callbacks; i++) {
             sched_start(callbacks[i], callback_args[i]);
        }
    }
};

#endif //LAUNCH_CORE_EVENT_H
