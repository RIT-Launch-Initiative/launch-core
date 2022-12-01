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

extern int8_t bmp3_init(bmp3_dev dev);

class BMP390 {
public:
    BMP390() {}

    RetType init() {
        bmp3_init(this->device);

        return
    }


private:
    bmp3_dev device;
    int8_t chip_id;


    void *intf_ptr;

    enum bmp3_intf commInterface;

    BMP3_INTF_RET_TYPE interfaceResult;
};

#endif //LAUNCH_CORE_BMP390_H
