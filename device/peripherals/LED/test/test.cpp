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

    printf("Initializing LED. Should be set to 1\n");
    LED led = LED(plainLED);
    led.init();

    printf("Initializing LED in OFF state. Should be set to 0\n");
    LED led2 = LED(plainLED, LED_OFF);
    led2.init();

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

    RGB rgb = RGB(redPin, greenPin, bluePin);

    printf("Initializing LED. Should be set to (255, 255, 255)\n");
    rgb.init();

    printf("Initializing LED with custom colors. Should be set to (33, 66, 99)\n");
    RGB rgb2 = RGB(redPin, greenPin, bluePin, 33, 66, 99);
    rgb2.init();

    printf("Toggling LED. Should be set to (0, 0, 0)\n");
    rgb.toggle();
    printf("Toggling LED. Should be set to (255, 255, 255)\n");
    rgb.toggle();

    printf("Setting LED values. Should be set to (0, 128, 255)\n");
    rgb.setValues(0, 128, 255);

    printf("Setting LED to red. (255, 0, 0)\n");
    rgb.setColor(RED);

    printf("Setting LED to green. (0, 255, 0)\n");
    rgb.setColor(GREEN);

    printf("Setting LED to blue. (0, 0, 255)\n");
    rgb.setColor(BLUE);

    printf("Setting LED to white. (255, 255, 255)\n");
    rgb.setColor(WHITE);



    return 0;
}