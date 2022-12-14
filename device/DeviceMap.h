/*******************************************************************************
*
*  Name: DeviceMap.h
*
*  Purpose: Maps devices to string names, there should be one device map for
*           a specific hardware platform.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef DEVICE_MAP_H
#define DEVICE_MAP_H

#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "return.h"
#include "device/Device.h"

/// @brief maps devices to names
/// this class is intended to be inherited to a platform specific map
class DeviceMap {
public:
    /// @brief get a device from the map with a specific name
    /// @param name     the name of the device
    /// @return         a pointer to the device, or NULL on error
    Device* get(const char* name) {
        for(size_t i = 0; i < m_count; i++) {
            if(!strcmp(name, m_names[i])) {
                // this is our device
                return m_devices[i];
            }
        }

        return NULL;
    }

    /// @brief initialize all devices added to the map
    /// @return error if any errors are returned, all devices are still initialized
    virtual RetType init() {
        RetType ret = RET_SUCCESS;

        for(size_t i = 0; i < m_count; i++) {
            if(RET_SUCCESS != m_devices[i]->init()) {
                ret = RET_ERROR;
            }
        }

        return ret;
    }

    /// @brief poll all the devices in the table
    /// @return error if any devices errored, will still poll all devices
    virtual RetType poll() {
        RetType ret = RET_SUCCESS;

        for(size_t i = 0; i < m_count; i++) {
            if(RET_SUCCESS != m_devices[i]->poll()) {
                ret = RET_ERROR;
            }
        }

        return ret;
    }

    #ifdef DEBUG\
    /// @brief use 'printf' to output a textual representation of the device map
    void print() {
        printf("%s\r\n", m_name);
        printf("------------------------------------------------\r\n");
        for(size_t i = 0; i < m_count; i++) {
            m_devices[i]->print();
        }
        printf("------------------------------------------------\r\n");
    }
    #endif

protected:
    /// @brief protected constructor
    /// use the constructor for the alloc::DeviceMap instead
    DeviceMap(const char* name, const char** names, Device** devices, size_t size) :
                                                        m_name(name),
                                                        m_names(names),
                                                        m_devices(devices),
                                                        m_size(size),
                                                        m_count(0) {};

    /// @brief add a socket device to the map
    ///        this should be called by derived classes to setup the map in 'init'
    /// @param name     the name of the device
    /// @param dev      the device to add
    /// @return
    RetType add(const char* name, Device* dev) {
        if(m_count >= m_size) {
            return RET_ERROR;
        }

        m_names[m_count] = name;
        m_devices[m_count] = dev;
        m_count++;

        return RET_SUCCESS;
    }
protected:
    // name of the device map
    const char* m_name;

    // list of string names
    const char** m_names;

    // list of device pointers
    Device** m_devices;

    // number of name/device pairs
    size_t m_size;

    // count of devices in the table
    size_t m_count;
};

namespace alloc {

/// @brief preallocated device map
/// @tparam SIZE    the number of entries in the map
template <const size_t SIZE>
class DeviceMap : public ::DeviceMap {
public:
    /// @brief constructor
    /// @param name     the name of the device map
    DeviceMap(const char* name) : ::DeviceMap(name, m_internalNames, m_internalDevices, SIZE) {};
private:
    const char* m_internalNames[SIZE];
    Device* m_internalDevices[SIZE];
};
}

#endif
