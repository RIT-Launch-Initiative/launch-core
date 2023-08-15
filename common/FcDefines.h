/**
 * Define variables to be shared across flight computer modules
 *
 * @author Aaron Chan
 */

#ifndef FC_DEFINES_H
#define FC_DEFINES_H

typedef enum {
    AUTOPILOT_MODULE = 0,
    POWER_MODULE,
    RADIO_MODULE,
    SENSOR_MODULE
} BackplaneModule;

/*******************************
 * Networking Definitions
 *******************************/

// TODO: Finalize standards


#endif // FC_DEFINES_H
