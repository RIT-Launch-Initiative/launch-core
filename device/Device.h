#ifndef DEVICE_H
#define DEVICE_H

/// @brief generic device
class Device {
public:
    /// @brief initialize the device
    /// @return 'true' on success, 'false' on failure
    virtual bool init() = 0;

    /// @brief obtain the device
    /// @return 'true' on success, 'false' on failure
    virtual bool obtain() = 0;

    /// @brief release the device
    /// @return 'true' on success, 'false' on failure
    virtual bool release() = 0;
};

#endif
