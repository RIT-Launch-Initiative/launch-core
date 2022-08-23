#ifndef LINUX_DEBUG_DEVICE_H
#define LINUX_DEBUG_DEVICE_H

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "device/Device.h"
#include "device/StreamDevice.h"

class LinuxDebugDevice : public StreamDevice {
public:
    LinuxDebugDevice() : StreamDevice("Linux Debug Device") {};

    RetType init() {
        // nothing to do since stderr is already open for us
        return RET_SUCCESS;
    }

    RetType obtain() {
        return RET_SUCCESS;
    }

    RetType release() {
        return RET_SUCCESS;
    }

    RetType poll() {
        // do nothing

        return RET_SUCCESS;
    }

    RetType write(uint8_t* buff, size_t len) {
        if(-1 == ::write(2, buff, len)) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    RetType read(uint8_t* buff, size_t len) {
        // reads always fail
        if(len > 0) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    size_t available() {
        return 0;
    }

    RetType wait(size_t len) {
        // will never be able to read any data
        return RET_ERROR;
    }
};

#endif
