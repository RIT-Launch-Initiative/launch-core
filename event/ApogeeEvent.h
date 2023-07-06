/*******************************************************************************
*
*  Name: ApogeeEvent.h
*
*  Purpose: Handle detecting an apogee event
*
*  Author: Aaron Chan
*
*  RIT Launch Initiative
*
*******************************************************************************/

#ifndef LAUNCH_CORE_APOGEEEVENT_H
#define LAUNCH_CORE_APOGEEEVENT_H

#include <stdint.h>

#include "return.h"
#include "event/Event.h"
#include "return.h"
#include "sched.h"

class ApogeeEvent : public Event {
public:
    ApogeeEvent(const bool *p_apogee_event_detected, uint16_t const *p_current_altitude) : Event(p_apogee_event_detected), m_highest_altitude(*p_current_altitude), p_current_altitude(p_current_altitude) {}

    RetType calculate_event() override {
        RESUME();

        RetType ret = RET_SUCCESS;
        if (*p_current_altitude > m_highest_altitude) {
            m_highest_altitude = *p_current_altitude;
            m_count++;
        } else if (*p_current_altitude < m_highest_altitude) {
            count++;

            if (count >= DETECT_COUNT) {
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
    uint16_t const *p_current_altitude;
    uint16_t m_highest_altitude = 0;
    uint8_t m_count = 0;
};

#endif //LAUNCH_CORE_APOGEEEVENT_H
