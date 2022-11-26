/**
* Test functionality of LED classes
 *
 * @author Aaron Chan
*/

#include "device/peripherals/LED/LED.h"
#include "device/peripherals/LED/RGB.h"
#include <stdio.h>

class LED_GPIO : public GPIODevice {
public:
    LED_GPIO(const char *name) : GPIODevice(name) {}

    RetType set(uint32_t val) {
        this->value = val;

        printf("\tSet to %d\n", val);

        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) {
        *val = this->value;

        return RET_SUCCESS;
    }

    RetType init() { return RET_SUCCESS; }

    RetType obtain() { return RET_SUCCESS; }

    RetType release() { return RET_SUCCESS; }

    RetType poll() { return RET_SUCCESS; }

private:
    uint32_t value;
};

int main(int argc, char **argv) {
    printf("BEGINNING BASIC LED TESTS\n");
    printf("-----------------------------\n");
    LED_GPIO plainLED = LED_GPIO("plain");
    LED led = LED(plainLED);
    printf("Initializing LED. Should be set to 1\n");
    led.init();

    printf("Toggling LED. Should be set to 0\n");
    led.toggle();

    printf("Toggling LED. Should be set to 1\n");
    led.toggle();

    printf("Setting LED to 0 (OFF)\n");
    led.setState(LED_OFF);

    printf("Setting LED to 1 (ON)\n");
    led.setState(LED_ON);

    printf("\nBEGINNING RGB LED TESTS\n");
    printf("-----------------------------\n");
    LED_GPIO redPin = LED_GPIO("red");
    LED_GPIO greenPin = LED_GPIO("green");
    LED_GPIO bluePin = LED_GPIO("blue");





    return 0;
}