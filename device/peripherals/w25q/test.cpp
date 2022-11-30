/**
* Test to ensure WQ25 functions won't die
*/

#include <stdio.h>
#include "device/peripherals/w25q/w25q.h"

class FakeSPIDevice : public SPIDevice {
public:
    FakeSPIDevice() : SPIDevice("FakeSpi") {

    }

    RetType write(uint8_t *data, size_t size) {
        RESUME();

        printf("Data:\n\t");
        for (int i = 0; i < size; i++) {
            printf("%d ", *(data + i));
        }
        printf("\n");

        RESET();
        return RET_SUCCESS;
    }

    RetType read(uint8_t *data, size_t size) {
        RESUME();
        uint8_t *start = data;
        for (int i = 0; i < size; i++) {
            *data++ = i;
        }
        *data = *start;

        RESET();
        return RET_SUCCESS;
    }

    RetType init() { return RET_SUCCESS; }

    RetType poll() { return RET_SUCCESS; }

    RetType obtain() { return RET_SUCCESS; }

    RetType release() { return RET_SUCCESS; }

};

class FakeGPIODevice : public GPIODevice {
public:
    /// @brief constructor
    FakeGPIODevice() : GPIODevice("FakeGPIODevice") {};

    /// @brief set the pin
    ///        this depends on what kind of GPIO it is
    ///        e.g. a digital can only be set to 0 or 1, but a PWM could be 0-255
    ///        we pass in a uint32_t to be safe
    ///        on input pins this may not work
    /// @param val      the value to set the pin to
    /// @return
    RetType set(uint32_t val) {
        RESUME();
        printf("Set to %d\n", val);

        RESET();
        return RET_SUCCESS;
    }

    /// @brief get the current value of the pin
    ///        value depends on what kind of pin it is
    ///        on output pins this may not work
    /// @param val      where to store the current value of the pin
    RetType get(uint32_t *val) {
        RESUME();
        *val = 0;

        RESET();
        return RET_SUCCESS;
    }

    RetType init() { return RET_SUCCESS; }

    RetType poll() { return RET_SUCCESS; }

    RetType obtain() { return RET_SUCCESS; }

    RetType release() { return RET_SUCCESS; }
};

int main() {
    FakeSPIDevice spiDev = FakeSPIDevice();
    FakeGPIODevice csPin = FakeGPIODevice();
    FakeGPIODevice clkPin = FakeGPIODevice();
    W25Q w25q = W25Q(spiDev, csPin, clkPin);

    printf("Testing init\n");
    w25q.init();

    printf("Testing toggle write\n");
    w25q.toggleWrite(WRITE_SET_ENABLE);

    uint8_t buff[256];
    uint8_t registerVal = 0;

    printf("Testing read register\n");
    w25q.readRegister(REGISTER_ONE_READ, &registerVal, 256);

    printf("Testing write register\n");
    w25q.writeRegister(REGISTER_ONE_WRITE, registerVal);
//    w25q.readData();
//    w25q.writeData();
//    w25q.eraseData();

    return 0;


}