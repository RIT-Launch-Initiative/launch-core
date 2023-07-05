/*******************************************************************************
*
*  Name: event_detection.h
*
*  Purpose: Handle detecting bosot events
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

class BoostEvent : public Event {
public:
    BoostEvent(const bool *p_boost_event_detected) : Event(p_boost_event_detected) {};

    RetType calculate_event(int16_t current_accel, int16_t current_altitude) override {
        RESUME();

        RetType ret = RET_SUCCESS;

        if (current_accel > m_avg_accel && current_altitude > m_avg_altitude) {
            m_count++;
        } else {
            m_count = 0;
        }

        if (m_count >= DETECT_COUNT) {
            *p_event_detected = true;

            ret = CALL(call_hooks());
        }

        RESET();
        return ret;
    };

private:
    int16_t m_avg_accel;

    int16_t m_avg_altitude;

    uint8_t m_count;
};

#endif //LAUNCH_CORE_BOOSTEVENT_H
