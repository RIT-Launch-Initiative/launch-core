#ifndef LINUX_MAP_H
#define LINUX_MAP_H

#include "device/DeviceMap.h"
#include "sched/macros.h"
#include "device/linux/ConsoleDevice.h"

static const size_t MAP_SIZE = 10;

class LinuxDeviceMap : public alloc::DeviceMap<MAP_SIZE> {
public:
    /// @brief constructor
    LinuxDeviceMap() : m_console() {};

    /// @brief initialize the Linux platform specific map
    RetType init() {
        RESUME();
        static RetType ret;

        ret = RET_SUCCESS;

        ret = CALL(add("console", &m_console));
        if(RET_ERROR == ret) {
            return ret;
        } // SLEEP and BLOCK are handled by CALL

        // TODO more devices

        RESET();
        return alloc::DeviceMap<MAP_SIZE>::init();
    }
private:
    // console device
    LinuxConsoleDevice m_console;
};

#endif
