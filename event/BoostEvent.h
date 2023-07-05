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

class BoostEvent : public Event {
public:
    const uint8_t DETECT_COUNT = 5;

    BoostEvent(*p_boost_event_detected) : Event(p_boost_event_detected) {}

    RetType calculate_event() override {
        RESUME();

        if (m_accel > m_avg_accel && m_altitude > m_avg_altitude) {
            m_count++;
        } else {
            m_count = 0;
        }

        if (m_count >= DETECT_COUNT) {
            *p_event_detected = true;

            RetType ret = CALL(call_hooks());
        }

        RESET();
        return ret;
    };

private:
    int16_t m_accel;
    int16_t m_avg_accel;

    int16_t m_altitude;
    int16_t m_avg_altitude;

    uint16_t m_count;
};

#endif //LAUNCH_CORE_BOOSTEVENT_H
