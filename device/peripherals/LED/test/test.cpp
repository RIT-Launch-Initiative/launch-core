/**
* Test functionality of LED classes
 *
 * @author Aaron Chan
*/

#include "device/peripherals/LED/LED.h"
#include "device/peripherals/LED/RGB.h"


class LED_GPIO : GPIODevice {
public:
    LED_GPIO(const char *name) : GPIODevice(name) {}

    RetType set(uint32_t val) {
        this->value = val;

        return RET_SUCCESS;
    }

    RetType get(uint32_t *val) {
        *val = this->value;

        return RET_SUCCESS;
    }

private:
    uint32_t value;
};

int main(int argc, char **argv) {

    return 0;
}