/*******************************************************************************
*
*  Name: DeviceHandlerTask.h
*
*  Purpose: Function that can be called as a task to repeatedly poll a device.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef DEVICE_HANDLER_TASK_H
#define DEVICE_HANDLER_TASK_H

#include "sched/macros.h"

/// @brief handle a device, essentially calls the 'poll' function of 'dev'
/// @param dev     a Device class pointer to the device to poll
/// @return
void HandleDevice(void* dev) {
    RESUME();

    // returns blocked or yield to return back to the scheduler
    RetType ret = CALL(dev->poll());

    // in cases where we get an error, don't want to stop handling the device.
    // hopefully this doesn't happen, but if it does just pretend everything is
    // fine so this task gets scheduled again.
    if(RET_ERROR == ret) {
        RESET();
        return RET_SUCCESS;
    }

    RESET();
    return ret;
}

#endif
