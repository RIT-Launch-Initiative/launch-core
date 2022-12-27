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

class RFM95W {
public:
    RFM95W(SPIDevice *spiDevice) : mSpi(spiDevice) {}

    RetType init() {
        RESUME();

//        RetType ret = ;
//        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

private:
    SPIDevice *mSpi;

};

#endif //LAUNCH_CORE_RFM95W_H
