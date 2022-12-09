/**
 * Implementation of the BMP390 device
 * by providing an interface for the BMP API
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_BMP390_H
#define LAUNCH_CORE_BMP390_H

#include "device/peripherals/bmp390/bmp3.h"
#include "return.h"


class BMP390 {
public:
    BMP390() {}

    RetType init() {
        int8_t result = bmp3_init(&this->device);

        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }

    RetType


private:
    bmp3_dev device;
    int8_t chip_id;


    void *intf_ptr;

    enum bmp3_intf commInterface;

    BMP3_INTF_RET_TYPE interfaceResult;

    RetType bmpResultConvert(int8_t result) {
        return result == 0 ? RET_SUCCESS : RET_ERROR;
    }
};

#endif //LAUNCH_CORE_BMP390_H
