/*******************************************************************************
*
*  Name: init.h
*
*  Purpose: Contains the common init function for a launch-core project.
*           This function should be the first function added to the scheduler.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/

#include <stdlib.h>

#include "sched/macros.h"
#include "device/DeviceMap.h"
#include "device/Device.h"

/// @brief helper function to poll a device, essentially just calls the 'poll'
///        function of 'dev'
/// @param dev     a Device class pointer to the device to poll
/// @return
void PollDevice(void* dev) {
    RESUME();

    // returns blocked or yield to return back to the scheduler
    RetType ret = CALL(((Device*)dev)->poll());

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

/// Argument passed to the init task
typedef struct {
    DeviceMap* dev_map;     // platform device map
    task_func_t* tasks;     // list of tasks to
    void** args;            // list of arguments passed to these tasks
    size_t num_tasks;       // number of tasks passed in the list
} init_arg_t;

/// @brief the init task! this should be the first task to be added to the
///        scheduler. It needs to be passed a device map and a list of tasks.
///
///        What it does:
///         - for each device in the device map, it inits the device and then
///           schedules a task to poll the device after it's inited.
///         - it adds each task in the task list to the scheduler AFTER all
///           devices are init'd. These are the first 'user' tasks to run after
///           the board is initialized.
///         - loops forever
///
/// @param init_args      the init arguments, an init_arg_t* cast to void*
void init(void* init_args) {
    RESUME();

    init_arg_t args = (init_arg_t*)init_args;

    DeviceMap* map = args->dev_map;
    Device* dev = map->next();

    // iterate through all the devices
    while(dev != NULL) {
        RetType ret = CALL(dev->init());

        if(RET_SUCCESS == ret) {
            // add a handler for this device
            // NOTE: not checking the return here, nothing we can really do
            //       if it fails
            sched_start(PollDevice, (void*)dev);
        }
        // otherwise this device failed to init, don't add it's handler to
        // the scheduler

        dev = map->next();
    }

    // start all the tasks we were passed
    for(size_t i = 0; i < args->num_tasks; i++) {
        task_func_t* task = args->tasks[i];
        void* arg = args->args[i];

        // NOTE: again not checking return here
        sched_start(task, arg);
    }

    // spin forever
    while(1) {};
}
