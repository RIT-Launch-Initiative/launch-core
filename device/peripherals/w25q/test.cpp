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

    uint8_t buff[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t registerVal = 0;

    printf("Testing write register\n");
    w25q.writeRegister(REGISTER_ONE_WRITE, 1);

    printf("Testing read register\n");
    w25q.readRegister(REGISTER_ONE_READ, &registerVal);
    printf("\tRegister value: %d\n", registerVal);


    printf("Testing write data\n");
    uint8_t newBuff[256];
    for (int i = 255; i > -1; i--) {
        newBuff[i] = i;
    }

    w25q.writeData(PAGE_PROGRAM, 0b000000, newBuff, 256);

    printf("Testing read data\n");
    w25q.readData(READ_DATA, 0b000000, buff, buff, 256); // 3 0 0 0
    for (uint8_t i : buff) {
        printf("%d ", i);
    }
    printf("\n");

    printf("Testing erase data\n");
    w25q.eraseData(SECTOR_ERASE, 0b000000);

    return 0;
}
