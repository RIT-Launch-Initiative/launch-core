/*******************************************************************************
*
*  Name: LandingEvent.h
*
*  Purpose: Handle detecting a landing event
*
*  Author: Aaron Chan
*
*  RIT Launch Initiative
*
*******************************************************************************/

#ifndef LAUNCH_CORE_LANDINGEVENT_H
#define LAUNCH_CORE_LANDINGEVENT_H

#include <stdint.h>
#include <math.h>

#include "return.h"
#include "event/Event.h"
#include "return.h"
#include "sched.h"
#include "sched/macros.h"

class LandingEvent : public Event {
public:
    LandingEvent(bool *const p_landing_event_detected, bool *const p_past_event, int16_t *const p_current_accel, int16_t *const p_current_altitude) : Event(p_landing_event_detected), m_avg_altitude(*p_current_altitude),
                                                                                                                                                      p_current_altitude(p_current_altitude), p_current_accel(p_current_accel),
                                                                                                                                                      p_past_event(p_past_event) {}

    RetType calculate_event() override {
        RESUME();

        RetType ret = RET_SUCCESS;
        if (*p_past_event && is_accel_in_range() &&  is_altitude_in_range()) {
            m_count++;

            m_avg_altitude = (m_avg_altitude + *p_current_altitude) / 2;

            if (m_count >= DETECT_COUNT) {
                *p_event_detected = true;
                call_hooks();
            }
        } else {
            m_count = 0;
        }

        RESET();
        return ret;
    };

private:
    int16_t m_avg_altitude{};
    int16_t *const p_current_altitude;
    int16_t *const p_current_accel;
    uint8_t m_count = 0;
    bool *const p_past_event;

    static constexpr uint8_t THREE_G_ACCEL = 9.81 * 3;

    [[nodiscard]] bool is_accel_in_range() const {
        return *p_current_accel < THREE_G_ACCEL;
    }

    [[nodiscard]] bool is_altitude_in_range() const {
        const uint8_t threshold_range = 50;
        const int16_t altitude_diff = *p_current_altitude - m_avg_altitude;

        return altitude_diff < threshold_range;
    }
};

#endif //LAUNCH_CORE_LANDINGEVENT_H
