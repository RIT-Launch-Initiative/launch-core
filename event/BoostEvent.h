/*******************************************************************************
*
*  Name: event_detection.h
*
*  Purpose: Handles detecting a boost event
*
*  Author: Aaron Chan
*
*  RIT Launch Initiative
*
*******************************************************************************/

#ifndef LAUNCH_CORE_BOOSTEVENT_H
#define LAUNCH_CORE_BOOSTEVENT_H

#include <stdint.h>

#include "return.h"
#include "event/Event.h"
#include "return.h"
#include "sched.h"
#include "sched/macros.h"


class BoostEvent : public Event {
public:
    BoostEvent(bool *const p_boost_event_detected, int16_t *const p_current_accel, int16_t *const p_current_altitude) : Event(p_boost_event_detected),
                                                                                                                        m_avg_accel(*p_current_accel), m_avg_altitude(*p_current_altitude),
                                                                                                                        p_current_accel(p_current_accel), p_current_altitude(p_current_altitude),
                                                                                                                        m_count(0) {};

    RetType calculate_event() override {
        RESUME();

        RetType ret = RET_SUCCESS;

        if (*p_current_accel > m_avg_accel && *p_current_altitude > m_avg_altitude) {
            m_avg_accel = (m_avg_accel + *p_current_accel) / 2;
            m_avg_altitude = (m_avg_altitude + *p_current_altitude) / 2;
            m_count++;

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
    int16_t m_avg_accel;
    int16_t m_avg_altitude;

    int16_t *const p_current_accel;
    int16_t *const p_current_altitude;

    uint8_t m_count{};
};

#endif //LAUNCH_CORE_BOOSTEVENT_H
