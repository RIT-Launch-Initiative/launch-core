/**
 * Platform independent implementation of TMP117 Sensor
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_TMP117_H
#define LAUNCH_CORE_TMP117_H

#include <stdint.h>

#define DEVICE_ID_VALUE 0x0117
#define TMP117_RESOLUTION 0.0078125f

typedef union {
    struct {
        uint8_t EMPTY: 1;            // Empty bit in register
        uint8_t TMP_SOFT_RESET: 1; // Software reset bit
        uint8_t DR_ALERT: 1;        // ALERT pin select bit
        uint8_t POL: 1;            // ALERT pin polarity bit
        uint8_t T_NA: 1;            // Therm/alert mode select
        uint8_t AVG: 2;            // Conversion averaging modes
        uint8_t CONV: 3;            // Conversion cycle bit
        uint8_t MOD: 2;            // Set conversion mode
        uint8_t EEPROM_BUSY: 1;    // EEPROM busy flag
        uint8_t DATA_READY: 1;        // Data ready flag
        uint8_t LOW_ALERT: 1;        // Low Alert flag
        uint8_t HIGH_ALERT: 1;        // High Alert flag
    } CONFIGURATION_FIELDS;
    uint16_t CONFIGURATION_COMBINED;
} CONFIGURATION_REG;

// Device ID Register used for checking if the device ID is the same as declared
typedef union {
    struct {
        uint16_t DID: 12; // Indicates the device ID
        uint8_t REV: 4;   // Indicates the revision number
    } DEVICE_ID_FIELDS;
    uint16_t DEVICE_ID_COMBINED;
} DEVICE_ID_REG;

enum TMP117_Register {
    TMP117_TEMP_RESULT = 0X00,
    TMP117_CONFIGURATION = 0x01,
    TMP117_T_HIGH_LIMIT = 0X02,
    TMP117_T_LOW_LIMIT = 0X03,
    TMP117_EEPROM_UL = 0X04,
    TMP117_EEPROM1 = 0X05,
    TMP117_EEPROM2 = 0X06,
    TMP117_TEMP_OFFSET = 0X07,
    TMP117_EEPROM3 = 0X08,
    TMP117_DEVICE_ID = 0X0F
};

enum TMP117_Mode {
    CONTINUOUS_CONVERSION = 0b00,
    ONE_SHOT = 0b11,
    SHUTDOWN = 0b01
};


class TMP117 {


};

#endif //LAUNCH_CORE_TMP117_H
