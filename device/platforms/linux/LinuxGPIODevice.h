/**
 * Implements a GPIO Device for Linux
 *
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_LINUXGPIODEVICE_H
#define LAUNCH_CORE_LINUXGPIODEVICE_H

#include <linux/gpio.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <cstdio>


#include "sched/macros/macros.h"
#include "device/GPIODevice.h"
#include "sync/BlockingSemaphore.h"

class LinuxGPIODevice : public GPIODevice {
public:
    LinuxGPIODevice(const char *name) : GPIODevice(name),
                                        taskLock(1),
                                        currentBlocked(-1) {};

    RetType init() override {
        RESUME();

        fd = open(m_name, O_RDWR);

#ifdef  DEBUG
        printf("fd: %d", fd);
#endif
        RESET();
        return fd == 0 ? RET_SUCCESS : RET_ERROR;;
    }

    RetType obtain() override {
        return RET_SUCCESS;
    }

    RetType poll() override {
        return RET_SUCCESS;
    }

    RetType release() override {
        return RET_SUCCESS;
    }

    RetType set(uint32_t val) override {
        RESUME();

        RetType ret = CALL(taskLock.acquire());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        taskLock = sched_dispatched;

        if (!(val == 0 || val == 1)) {
            RESET();

            return RET_ERROR;
        }

        if (ioctl(fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &val) == 0) {
            return RET_ERROR;
        }
        BLOCK();

        currentBlocked = -1;

        ret = CALL(taskLock.release());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) override {
        RESUME();

        RetType ret = CALL(taskLock.acquire());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        taskLock = sched_dispatched;
        if (ioctl(fd, GPIO_V2_LINE_GET_VALUES_IOCTL, val) == 0) {
            return RET_ERROR;
        }

        BLOCK();

        currentBlocked = -1;

        ret = CALL(taskLock.release());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

private:
    BlockingSemaphore taskLock;
    tid_t currentBlocked;
    int fd = -1;


};

#endif //LAUNCH_CORE_LINUXGPIODEVICE_H
