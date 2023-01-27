/**
 * LSM6DSL IMU Facade
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_LSM6DSL_H
#define LAUNCH_CORE_LSM6DSL_H

#include <stdint.h>
#include "device/I2CDevice.h"

class LSM6DSL {
public:
    LSM6DSL(I2CDevice* i2CDevice) : mI2C(i2CDevice) {

    }

private:
    I2CDevice *mI2C;

};

/* Imported function prototypes ----------------------------------------------*/
uint8_t LSM6DSL_IO_Write(void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite) {

    return 0;
}

uint8_t LSM6DSL_IO_Read(void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead) {


    return 0;
}

#endif //LAUNCH_CORE_LSM6DSL_H
