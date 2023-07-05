/*******************************************************************************
*
*  Name: event_detection.h
*
*  Purpose: Tasks meant to determine flight events
*
*  Author: Aaron Chan
*
*  RIT Launch Initiative
*
*******************************************************************************/

#ifndef LAUNCH_CORE_EVENT_DETECTION_H
#define LAUNCH_CORE_EVENT_DETECTION_H

namespace event_detection {
    RetType boost_detection(bool *p_boost_detected, ) {
        RESUME();


        RESET();
        return RET_SUCCESS;
    };

    RetType apogee_detection(bool *p_apogee_detected, ) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    };

    RetType landing_detection(bool *p_landing_detected, ) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    };
}

#endif //LAUNCH_CORE_EVENT_DETECTION_H
