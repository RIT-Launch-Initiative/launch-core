/**
 * RFM95W LoRa Driver
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_RFM95W_H
#define LAUNCH_CORE_RFM95W_H

#include "return.h"
#include "sched/macros/resume.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "device/SPIDevice.h"
#include "device/StreamDevice.h"

class RFM95W : public StreamDevice {
public:
    RFM95W(SPIDevice *spiDevice) : StreamDevice("RFM95W"), mSpi(spiDevice) {}

    RetType init() {
        RESUME();

//        RetType ret = ;
//        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType write(uint8_t* buff, size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    RetType read(uint8_t* buff, size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    size_t available() {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    RetType wait(size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

private:
    SPIDevice *mSpi;

};

#endif //LAUNCH_CORE_RFM95W_H
