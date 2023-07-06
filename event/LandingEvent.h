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

class LandingEvent : public Event {
public:
    LandingEvent(bool const *p_landing_event_detected, bool const *p_past_event, int16_t const *p_current_accel, int16_t const *p_current_altitude) : Event(p_landing_event_detected), p_past_event(p_past_event),
                                                                                                                                                      p_current_accel(p_current_accel), p_current_altitude(p_current_altitude) {}

    RetType calculate_event() override {
        RESUME();

        RetType ret = RET_SUCCESS;
        if (*p_past_event && is_accel_in_range() &&  is_altitude_in_range()) {
            m_count++;

            m_avg_altitude = (m_avg_altitude + current_altitude) / 2;

            if (m_count >= DETECT_COUNT) {
                *p_event_detected = true;
                ret = CALL(call_hooks());
            }
        } else {
            m_count = 0;
        }

        RESET();
        return ret;
    };

private:
    bool const *p_past_event;
    uint16_t m_avg_altitude;
    uint16_t const *p_current_altitude;
    uint16_t const *p_current_accel;
    uint8_t m_count = 0;

    constexpr uint8_t THREE_G_ACCEL = 9.81 * 3;

    bool is_accel_in_range() {
        return *p_current_altitude < threshold_range;
    }

    bool is_altitude_in_range() {
        const uint8_t threshold_range = 50;
        const uint8_t altitude_diff = *p_current_altitude - m_avg_altitude;

        return altitude_diff < threshold_range;
    }
};

#endif //LAUNCH_CORE_LANDINGEVENT_H
