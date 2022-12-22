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
    ADXL375(I2CDevice& i2c) : m_i2c(i2c)
        {}

   RetType readX(int16_t* xAxis){
        // TODO read the values of the MSB and LSB and pass it into the final variable
        RetType ret;
        RESUME();
        uint8_t lsb;
        uint8_t msb;
        uint16_t value;

        // reading the data
        ret = CALL(m_i2c.read(xLSBData, lsb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        ret = CALL(m_i2c.read(xMSBData, msb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        // combining the data
        value |= msb << 8;
        value |= lsb;

        // value is in 2's complement so have to convert it
        int16_t adjustedValue = value;
        adjustedValue *= -1;
        *xAxis = adjustedValue;

        RESET();
       return RET_SUCESS;
    }

    RetType readY(int16_t* yAxis){
        // TODO read the values of the MSB and LSB and pass it into the final variable
        RetType ret;
        RESUME();
        uint8_t lsb;
        uint8_t msb;
        uint16_t value;

        // reading the data
        ret = CALL(m_i2c.read(yLSBData, lsb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        ret = CALL(m_i2c.read(yMSBData, msb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        // combining the data
        value |= msb << 8;
        value |= lsb;

        // value is in 2's complement so have to convert it
        int16_t adjustedValue = value;
        adjustedValue *= -1;
        *yAxis = adjustedValue;

        RESET();
        return RET_SUCCESS;
    }

    RetType readZ(int16_t* zAxis){
        // TODO read the values of the MSB and LSB and pass it into the final variable
        RetType ret;
        RESUME();
        uint8_t lsb;
        uint8_t msb;
        uint16_t value;

        // reading the data
        ret = CALL(m_i2c.read(zLSBData, lsb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        ret = CALL(m_i2c.read(zMSBData, msb, 1));
        if (ret != RET_SUCCESS){
            RESET();
            return ret;
        }

        // combining the data
        value |= msb << 8;
        value |= lsb;

        // value is in 2's complement so have to convert it
        int16_t adjustedValue = value;
        adjustedValue *= -1;
        *zAxis = adjustedValue;

        RESET();
        return RET_SUCCESS;

    }


private:
    I2CDevice& m_i2c;

    // I2CAddr for the different axis data
    I2CAddr_t xLSBData;
    I2CAddr_t xMSBData;
    I2CAddr_t yLSBData;
    I2CAddr_t yMSBData;
    I2CAddr_t zLSBData;
    I2CAddr_t zMSBData;

    xLSBData.dev_addr = 0x3B;
    xLSBData.mem_addr = 0x32;
    xLSBData.mem_addr_size = 1;

    xMSBData.dev_addr = 0x3B;
    xMSBData.mem_addr = 0x33;
    xMSBData.mem_addr_size = 1;

    yLSBData.dev_addr = 0x3B;
    yLSBData.mem_addr = 0x34;
    yLSBData.mem_addr_size = 1;

    yMSBData.dev_addr = 0x3B;
    yMSBData.mem_addr = 0x35;
    yMSBData.mem_addr_size = 1;

    zLSBData.dev_addr = 0x3B;
    zLSBData.mem_addr = 0x36;
    zLSBData.mem_addr_size = 1;

    zMSBData.dev_addr = 0x3B;
    zMSBData.mem_addr = 0x37;
    zMSBData.mem_addr_size = 1;
};


#endif //LAUNCH_CORE_ADXL375_H
