#ifndef DEVICE_MAP_H
#define DEVICE_MAP_H

#include <stdlib.h>
#include <string.h>

#include "return.h"
#include "device/Device.h"

/// @brief maps devices to names
class DeviceMap {
public:

    /// @brief add a socket device to the map
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

    /// @brief poll all the devices in the table
    /// @return error if any devices errored, will still poll all devices
    RetType poll() {
        RetType ret = RET_SUCCESS;

        for(size_t i = 0; i < m_count; i++) {
            if(RET_SUCCESS != m_devices[i]->poll()) {
                ret = RET_ERROR;
            }
        }

        return ret;
    }

protected:
    /// @brief protected constructor
    /// use the constructor for the alloc::DeviceMap instead
    DeviceMap(const char** names, Device** devices, size_t size) :
                                                        m_names(names),
                                                        m_devices(devices),
                                                        m_size(size),
                                                        m_count(0) {};
private:
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
    DeviceMap() : ::DeviceMap(m_names, m_devices, SIZE) {};
private:
    const char* m_names[SIZE];
    Device* m_devices[SIZE];
};
}

#endif
