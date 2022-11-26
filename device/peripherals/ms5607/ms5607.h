/**
 * Implementation of driver for the MS5607 Altimeter
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_MS5607_H
#define LAUNCH_CORE_MS5607_H

#include "device/GPIODevice.h"

typedef enum {
    I2C_PROTOCOL = 1,
    SPI_PROTOCOL = 0
} MS5607_SERIAL_PROTOCOL_T;


class MS5607 {
public:
    MS5607(GPIODevice &psPin, GPIODevice &diPin, GPIODevice &doPin, GPIODevice &csPin, GPIODevice &sdaPin) :
            protocolSelectPin(psPin), dataInPin(diPin), dataOutPin(doPin), chipSelectPin(csPin), serialDataPin(sdaPin)
    {}

    setProtocol(MS5607_SERIAL_PROTOCOL_T protocol) {
        this->selectedProtocol = protocol;
    }

private:
    MS5607_SERIAL_PROTOCOL_T selectedProtocol;
    GPIODevice &protocolSelectPin;

    GPIODevice &dataInPin;
    GPIODevice &dataOutPin;
    GPIODevice &chipSelectPin;

    GPIODevice &serialDataPin;
};

#endif //LAUNCH_CORE_MS5607_H
