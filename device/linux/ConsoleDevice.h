#ifndef LINUX_CONSOLE_DEVICE_H
#define LINUX_CONSOLE_DEVICE_H

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "sched/macros.h"
#include "device/Device.h"
#include "device/StreamDevice.h"
#include "ringbuffer/ringbuffer.h"
#include "queue/allocated_queue.h"

class LinuxConsoleDevice : public StreamDevice {
public:
    LinuxConsoleDevice() : m_rxBuff(),
                           m_lock(false),
                           m_blockTid(-1),
                           m_blockLen(0),
                           StreamDevice("Linux Console Device") {};

    RetType init() {
        // nothing to do, stdin and stdout are already open for us
        return RET_SUCCESS;
    }

    // can only be obtained by one
    RetType obtain() {
        if(m_lock) {
            return RET_ERROR;
        }

        m_lock = true;
        return RET_SUCCESS;
    }

    RetType release() {
        // always succeed
        m_lock = false;
        return RET_SUCCESS;
    }

    RetType poll() {
        // check if there's any data on stdin
        // if there is, read a character and push it to the ringbuffer

        struct timeval tv = { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);

        if(select(1, &fds, NULL, NULL, &tv) > 0) {
            char c = getchar();
            if(1 != m_rxBuff.push((uint8_t*)&c, sizeof(char))) {
                return RET_ERROR;
            }

            if(m_blockTid != -1) {
                if(m_rxBuff.size() >= m_blockLen) {
                    WAKE(m_blockTid);
                    m_blockTid = -1; // TODO instead of -1 #define an invalid TID somewhere
                }
            }
        }

        return RET_SUCCESS;
    }

    RetType write(uint8_t* buff, size_t len) {
        if(-1 == ::write(1, buff, len)) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    RetType read(uint8_t* buff, size_t len) {
        if(m_rxBuff.size() >= len) {
            if(len != m_rxBuff.pop(buff, len)) {
                return RET_ERROR;
            }

            return RET_SUCCESS;
        }

        // otherwise block
        wait(len);
    }

    size_t available() {
        return m_rxBuff.size();
    }

    // wait for a specific amount of data
    // called a second time after caller is woken, must remember location and return success
    RetType wait(size_t len) {
        RESUME();

        m_blockLen = len;
        m_blockTid = sched_dispatched;

        BLOCK();

        RESET();
        return RET_SUCCESS;
    }

private:
    RingBuffer<256, true> m_rxBuff;
    bool m_lock;

    tid_t m_blockTid;
    size_t m_blockLen;
};

#endif
