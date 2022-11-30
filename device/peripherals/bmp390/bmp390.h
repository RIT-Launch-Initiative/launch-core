/**
 * Implementation of the BMP390 device
 * by providing an interface for the BMP API
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_BMP390_H
#define LAUNCH_CORE_BMP390_H

#include "bmp3_defs.h"

class BMP390 {
public:
    BMP390() {

    }


private:
    bmp3_dev device;
    int8_t chip_id;


    void *intf_ptr;

    enum bmp3_intf intf;

    BMP3_INTF_RET_TYPE intf_rslt;
};

#endif //LAUNCH_CORE_BMP390_H
