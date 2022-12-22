//
// Created by skippynose on 12/17/22.
//

#ifndef LAUNCH_CORE_ADXL375_H
#define LAUNCH_CORE_ADXL375_H


#include <stdlib.h>
#include <stdint.h>

#include "sched/macros.h"
#include "return.h"
#include "device/I2CDevice.h"


class ADXL375 {
public:
    ADXL375(I2CDevice& i2c) : m_i2c(i2c) {}

   RetType readX(int16_t* xAxis){
        RESUME();
        uint8_t* lsb;
        uint8_t* msb;

        // reading the data
        RetType ret = CALL(m_i2c.read(xLSBData, lsb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        ret = CALL(m_i2c.read(xMSBData, msb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        // value is in 2's complement so have to convert it
        *xAxis = ((msb << 8) | lsb) * -1;

        RESET();
        return RET_SUCCESS;
    }

    RetType readY(int16_t* yAxis){
        RESUME();
        uint8_t* lsb;
        uint8_t* msb;

        // reading the data
        RetType ret = CALL(m_i2c.read(yLSBData, lsb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        ret = CALL(m_i2c.read(yMSBData, msb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }


        // value is in 2's complement so have to convert it
        *yAxis = ((msb << 8) | lsb) * -1;

        RESET();
        return RET_SUCCESS;
    }

    RetType readZ(int16_t* zAxis){
        RESUME();
        uint8_t* lsb;
        uint8_t* msb;

        // reading the data
        RetType ret = CALL(m_i2c.read(zLSBData, lsb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        ret = CALL(m_i2c.read(zMSBData, msb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        *zAxis = ((msb << 8) | lsb) * -1;

        RESET();
        return RET_SUCCESS;

    }


private:
    I2CDevice& m_i2c;

    // I2CAddr for the different axis data
    I2CAddr_t xLSBData {0x3B, 0x32, 1};
    I2CAddr_t xMSBData {0x3B, 0x33, 1};
    I2CAddr_t yLSBData {0x3B, 0x34, 1};
    I2CAddr_t yMSBData {0x3B, 0x35, 1};
    I2CAddr_t zLSBData {0x3B, 0x36, 1};
    I2CAddr_t zMSBData {0x3B, 0x37, 1};
};


#endif //LAUNCH_CORE_ADXL375_H
