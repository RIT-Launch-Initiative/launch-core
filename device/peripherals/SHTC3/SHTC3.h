/**
 * Platform Independent Driver for the SHTC3 Sensor
 *
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_SHTC3_H
#define LAUNCH_CORE_SHTC3_H

#include "return.h"
#include "sched/macros/resume.h"
#include "sched/macros/reset.h"
#include "device/I2CDevice.h"


#define SHTC3_I2C_ADDR 0x70
#define SHTC3_NORMAL_MEAS_TFIRST_STRETCH 0x7CA2 /**< Normal measurement, temp first with Clock Stretch Enabled */
#define SHTC3_LOWPOW_MEAS_TFIRST_STRETCH 0x6458 /**< Low power measurement, temp first with Clock Stretch Enabled */
#define SHTC3_NORMAL_MEAS_HFIRST_STRETCH 0x5C24 /**< Normal measurement, hum first with Clock Stretch Enabled */
#define SHTC3_LOWPOW_MEAS_HFIRST_STRETCH 0x44DE /**< Low power measurement, hum first with Clock Stretch Enabled */

#define SHTC3_NORMAL_MEAS_TFIRST 0x7866 /**< Normal measurement, temp first with Clock Stretch disabled */
#define SHTC3_LOWPOW_MEAS_TFIRST 0x609C /**< Low power measurement, temp first with Clock Stretch disabled */
#define SHTC3_NORMAL_MEAS_HFIRST 0x58E0 /**< Normal measurement, hum first with Clock Stretch disabled */
#define SHTC3_LOWPOW_MEAS_HFIRST 0x401A /**< Low power measurement, hum first with Clock Stretch disabled */

#define SHTC3_READ_ID_CMD 0xEFC8    /**< Read Out of ID Register */
#define SHTC3_SOFT_RESET_CMD 0x805D /**< Soft Reset */
#define SHTC3_SLEEP_CMD 0xB098     /**< Enter sleep mode */
#define SHTC3_WAKEUP_CMD 0x3517    /**< Wakeup mode */

enum SHTC3_CMD {

};


class SHTC3 {
public:
    SHTC3(I2CDevice *i2CDevice) : mI2C(i2CDevice) {}

    RetType init() {

    }

    RetType writeCommand(SHTC3_CMD command16) {
        RESUME();

        uint8_t command8[2] = {};
        uint16ToUint8(command16, command8);

        RetType ret = mI2C->write(addr, command8, 2);
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType readCommand(SHTC3_CMD command16, uint8_t *buff, uint8_t numBytes) {
        RESUME();

        uint8_t command8[2] = {};
        uint16ToUint8(command16, command8);

        RetType ret = mI2C->write(addr, command8, 2);
        if (ret != RET_SUCCESS) return ret;

        ret = mI2C->read(addr, buff, numBytes);
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }


private:
    I2CDevice *mI2C;
    I2CAddr_t addr;

    int16_t uint8ToInt16(uint8_t *data) {
        return (data[0] << 8) | data[1];
    }

    void int16ToUint8(int16_t data16, uint8_t *data8) {
        data8[0] = static_cast<uint8_t>(data16 >> 8);
        data8[1] = static_cast<uint8_t>(data16 & 0xFF);
    }

    void uint16ToUint8(uint16_t data16, uint8_t *data8) {
        data8[0] = static_cast<uint8_t>(data16 >> 8);
        data8[1] = static_cast<uint8_t>(data16 & 0xFF);
    }

    static uint8_t crc8(const uint8_t *data, int len) {
        const uint8_t POLYNOMIAL(0x31);
        uint8_t crc(0xFF);

        for (int i = len; i; --i) {
            crc ^= *data++;

            for (int j = 8; j; --j) {
                crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
            }
        }
        return crc;
    }

};


#endif //LAUNCH_CORE_SHTC3_H
