/**
 * This is the device independent driver for the MS5607 Barometric sensor module.
 * The datasheet can be found at https://www.te.com/commerce/DocumentDelivery/DDEController?Action=srchrtrv&DocNm=MS5607-02BA03&DocType=Data+Sheet&DocLang=English
 **/

#ifndef MS5607_H
#define MS5607_H

#include <stdlib.h>
#include <stdint.h>

#include "device/StreamDevice.h"
#include "sched/macros.h"
#include "return.h"


class MS5607 {
public:
    // constructor method, here send a reset command to the chip, must happen once after power on
    // constructor must also read PROM and the stores the different 1-6 cofficients
    MS5607(I2CDevice &i2c) : m_i2c(i2c) {
        // RESET command sent to the device
        CALL(m_i2c.write(i2cWriteAddr, REST, 1)); // addr, 0xE1, 1 byte (8 bits)

        // The buffer to write variables into
        uint8_t *buff;

        // reading PROM coffients and storing them
        CALL(m_i2c.write(i2cWriteAddr, c1, 1)); // reading c1
        SENS_T1 = CALL(m_i2c.read(i2cReadAddr, buff, 2));

        CALL(m_i2c.write(i2cWriteAddr, c2, 1)); // reading c2
        OFF_T1 = CALL(m_i2c.read(i2cReadAddr, buff, 2));

        CALL(m_i2c.write(i2cWriteAddr, c3, 1)); // reading c3
        TCS = CALL(m_i2c.read(i2cReadAddr, buff, 2));

        CALL(m_i2c.write(i2cWriteAddr, c4, 1)); // reading c4
        TCO = CALL(m_i2c.read(i2cReadAddr, buff, 2));

        CALL(m_i2c.write(i2cWriteAddr, c5, 1)); // reading c5
        T_REF = CALL(m_i2c.read(i2cReadAddr, buff, 2));

        CALL(m_i2c.write(i2cWriteAddr, c6, 1)); // reading c6
        TEMP_SENS = CALL(m_i2c.read(i2cReadAddr, buff, 2));
    }

    // public methods to return compensated pressure and temperate based on converted (D1 and D2) data using cofficients read in from the PROM

    /**
     * @breif a function to return the calculated pressure from offset and D1
     */

    RetType pressure() {
        uint8_t *pressureBuffer;
        RetType pressureValue = read_pressure(i2cWriteAddr, i2cReadAddr, pressureBuffer);
        return pressureValue;
    }


private:

    StreamDevice &m_i2c;

    // Don't understand what it means by CSB value form datasheet, therefore leaving these variables undefined for now.
    uint16_t i2cReadAddr;
    uint16_t i2cWriteAddr;

    // PROM variables (QUESTIONS: Should these be RetTypes or uint16_t???)
    RetType SENS_T1;
    RetType OFF_T1;
    RetType TCS;
    RetType TCO;
    RetType T_REF;
    RetType TEMP_SENS;

    // PROM registors
    uint8_t c1 = 0xA2; // cofficient 1
    uint8_t c2 = 0xA4; // cofficient 2
    uint8_t c3 = 0xA6; // cofficinet 3
    uint8_t c4 = 0xA8; // cofficinet 4
    uint8_t c5 = 0xAA; // cofficinet 5
    uint8_t c6 = 0xAC; // cofficient 6

    // Commands
    uint8_t REST = 0x1E; // Reset command, need to call in the constructor
    uint8_t ADC = 0x00; // ADC read command
    uint8_t D1 = 0x48; // Temp convert
    uint8_t D2 = 0x58; // Pressure convert

    // Data buffers to store the data
    RetType d1Converter;
    RetType d2Converter;

    /**@breif a helper method to read D1 value and use PROM variables
     * to calculate pressure
     * @param WriteAddr the address to write to.
     * @param ReadAddr the adress to read from
     * @param buff, the buffer to write to
     * @return the final pressure
     */
    RetType read_pressure(uint8_t WriteAddr, uint8_t ReadAddr, uint8_t *buff) {
        RetType ret;
        RetType finalPressure;

        // Starting D1 conversion
        CALL(m_i2c.write(WriteAddr, D1, 1));

        // Writing to the sensor to start the ADC message
        CALL(m_i2c.write(WriteAddr, ADC, 1));

        // Reading the values from the sensor
        ret = CALL(m_i2c.read(ReadAddr, buff, 4));

        uint8_t *dTBuffer;
        // Getting the temp difference
        RetType dT = computeTempDT(WriteAddr, ReadAddr, dTBuffer);

        // Math to convert the digital value into an actual pressure
        RetType SEN = SENS_T1 * TCS * dT;
        finalPressure = D1 * SENS - OFF;

        // Return the value
        return finalPressure;


    }

    RetType computeTempDT(uint8_t WriteAddr, uint8_t ReadAddr, uint8_t *buff) {
        RetType ret;
        RetType dT;

        // starting D2 conversion
        CALL(m_i2c.write(WriteAddr, D2, 1));
        CALL(m_i2c.write(WriteAddr, ADC, 1));

        ret = CALL(m_i2c.read(ReadAddr, buff, 4));

        // computing D2 here
        dT = D2 - T_REF;
        return dT;

    }



    //private method to read uncompendsated temperature

    /**
     * List of things needed to be done after verfying that current code can
     * actually compile and work
     * TODO:
     *  1. Get temperature
     *  2. Compute Second order temperature compenstation*/


};

#endif //MS5607_H
