#ifndef LINUX_MAP_H
#define LINUX_MAP_H

// TODO this belongs in the project for the module
//      this is specific to the platform and peripherals so should not be in core
//      for now it's convenient to test with

#ifdef DEBUG
#include <stdio.h>
#endif

#include "device/DeviceMap.h"
#include "sched/macros.h"
#include "device/linux/ConsoleDevice.h"
#include "device/linux/DebugDevice.h"
#include "device/linux/LinuxUdpSocketDevice.h"


static const size_t MAP_SIZE = 10;

class LinuxDeviceMap : public alloc::DeviceMap<MAP_SIZE> {
public:
    /// @brief constructor
    LinuxDeviceMap() : m_console(),
                       m_debug(),
                       m_socket() {};

    /// @brief initialize the Linux platform specific map
    /// TODO put in cpp file
    RetType init() {
        // add our devices
        // NOTE: we don't error check here
        // if we add too many devices, it's our own fault
        // just make MAP_SIZE bigger if we need more
        add("console", &m_console);
        add("debug", &m_debug);
        add("net", &m_socket);

        // duplicate the console as our debug output

        return alloc::DeviceMap<MAP_SIZE>::init();
    }

    #ifdef DEBUG
    /// @brief use 'printf' to output a textual representation of the device map
    void print() {
        printf("Linux Test Device Map version 0.1\r\n");
        printf("------------------------------------------------\r\n");
        for(size_t i = 0; i < m_count; i++) {
            m_devices[i]->print();
        }
        printf("------------------------------------------------\r\n");
    }
    #endif

private:
    // console device
    LinuxConsoleDevice m_console;

    // debug device
    LinuxDebugDevice m_debug;

    // socket device
    LinuxUdpSocketDevice m_socket;
};

#endif
