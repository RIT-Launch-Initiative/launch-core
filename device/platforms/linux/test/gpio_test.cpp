#include <cstdio>
#include "device/platforms/linux/LinuxGPIODevice.h"

/**
* Test Linux GPIO Implementation
*/

int main() {
    LinuxGPIODevice gpio = LinuxGPIODevice("/dev/gpiochip0");
    RetType ret = gpio.init();

    if (ret != RET_SUCCESS) {
        printf("Error initializing GPIO\n");
        return 1;
    }

    uint32_t gpioState;
    while (true) {
        ret = gpio.get(&gpioState);

        if (ret != RET_SUCCESS) {
            printf("Error getting GPIO state: %d\n", ret);
        } else {
            printf("GPIO state: %d\n", gpioState);
            ret = gpio.set(gpioState == 0 ? 1 : 0);

            if (ret != RET_SUCCESS) {
                printf("Error setting GPIO state: %d\n", ret);
            }
        }

        sleep(1);
    }


    return 0;
}
