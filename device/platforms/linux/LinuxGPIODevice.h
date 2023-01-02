/**
 * Implements a GPIO Device for Linux
 *
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_LINUXGPIODEVICE_H
#define LAUNCH_CORE_LINUXGPIODEVICE_H

#include "sched/macros.h"
#include "device/GPIODevice.h"
#include "device/platforms/stm32/HAL_Handlers.h"
#include "sync/BlockingSemaphore.h"

class LinuxGPIODevice : public GPIODevice {
    LinuxGPIODevice(const char* name) : GPIODevice(name),
                                        taskLock(1),
                                        currentBlocked(-1),
                                        pin(0) {};

    RetType init() override {
        return RET_SUCCESS;
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

        // TODO: Set Pin
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
        *val = nullptr; // TODO

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
    uint16_t pin;
};

#endif //LAUNCH_CORE_LINUXGPIODEVICE_H
