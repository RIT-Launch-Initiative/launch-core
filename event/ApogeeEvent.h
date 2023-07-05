/*******************************************************************************
*
*  Name: ApogeeEvent.h
*
*  Purpose: Handle detecting bosot events
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
    ApogeeEvent(const bool *p_apogee_event_detected) : Event(p_apogee_event_detected) {}

    RetType calculate_event() override {
        RESUME();
        RetType ret = RET_SUCCESS;



        RESET();
        return ret;
    };

private:

    uint8_t m_count;
};

#endif //LAUNCH_CORE_APOGEEEVENT_H
