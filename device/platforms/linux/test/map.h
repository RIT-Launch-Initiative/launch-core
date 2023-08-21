#ifndef LINUX_MAP_H
#define LINUX_MAP_H

// TODO this belongs in the project for the module
//      this is specific to the platform and peripherals so should not be in core
//      for now it's convenient to test with

#include "device/DeviceMap.h"
#include "sched/macros/macros.h"
#include "device/platforms/linux/ConsoleDevice.h"
#include "device/platforms/linux/DebugDevice.h"
// #include "device/platforms/linux/LinuxUdpSocketPool.h"
#include "device/platforms/linux/LinuxBlockDevice.h"


static const size_t MAP_SIZE = 10;

class LinuxDeviceMap : public alloc::DeviceMap<MAP_SIZE> {
public:
    /// @brief constructor
    LinuxDeviceMap() : m_console(),
                       m_debug(),
                       // m_sockets("Linux Socket Pool"),
                       m_block("block_dev", 512, 1024),
                       alloc::DeviceMap<MAP_SIZE>("Linux Test Device Map") {};

    /// @brief initialize the Linux platform specific map
    /// TODO put in cpp file
    RetType init() {
        // add our devices
        // NOTE: we don't error check here
        // if we add too many devices, it's our own fault
        // just make MAP_SIZE bigger if we need more
        add("console", &m_console);
        add("debug", &m_debug);
        // add("socket_pool", &m_sockets);
        add("block", &m_block);

        return alloc::DeviceMap<MAP_SIZE>::init();
    }

private:
    // console device
    LinuxConsoleDevice m_console;

    // debug device
    LinuxDebugDevice m_debug;

    // socket pool
    // LinuxUdpSocketPool<10> m_sockets;

    // block device
    LinuxBlockDevice m_block;
};

#endif
