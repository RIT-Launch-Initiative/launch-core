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

        printf("Set to %d\n", val);

        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) {
        *val = this->value;

        return RET_SUCCESS;
    }

    RetType init() {return RET_SUCCESS;}
    RetType obtain() {return RET_SUCCESS;}
    RetType release() {return RET_SUCCESS;}
    RetType poll() {return RET_SUCCESS;}

private:
    uint32_t value;
};

int main(int argc, char **argv) {
    LED_GPIO plainLED = LED_GPIO("plain");
    LED led = LED(plainLED);


    return 0;
}