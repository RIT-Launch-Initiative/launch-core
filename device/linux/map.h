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
        // add our devices
        // NOTE: we don't error check here
        // if we add too many devices, it's our own fault
        // just make MAP_SIZE bigger if we need more
        add("console", &m_console)

        return alloc::DeviceMap<MAP_SIZE>::init();
    }
private:
    // console device
    LinuxConsoleDevice m_console;
};

#endif
