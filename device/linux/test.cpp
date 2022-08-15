#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "device/linux/map.h"
#include "device/StreamDevice.h"
#include "sched/sched.h"
#include "sched/macros.h"

// *** stolen from stack overflow to make terminal work *** //
// notable disables line buffering
// https://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input
struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}
// ******************************************************* //

LinuxDeviceMap map{};

// task that just echoes
// buffers 5 characters at a time
RetType echo() {
    RESUME();

    printf("task started\r\n");

    Device* dev = map.get("console");
    if(dev == NULL) {
        return RET_ERROR;
    }

    static StreamDevice* console = reinterpret_cast<StreamDevice*>(dev);

    RetType ret;

    while(1) {
        ret = CALL(console->wait(5));

        if(RET_ERROR == ret) {
            continue;
        }

        if(console->available() < 5) {
            printf("task woken but console has less than 5 bytes available\r\n");
            YIELD();
        }

        uint8_t buff[7];

        // returns on anything other than success
        ret = console->read(buff, 5);
        if(ret != RET_SUCCESS) {
            continue;
        }

        buff[5] = '\r';
        buff[6] = '\n';
        console->write(buff, 8); // don't care if it works

        // sleep for a thousand ticks
        SLEEP(1000);
    }

    // should never get here
    return RET_ERROR;
}

uint32_t tick = 0;
uint32_t systime() {
    usleep(1000);

    tick++;
    return tick;
}

int main() {
    set_conio_terminal_mode();

    if(RET_SUCCESS != map.init()) {
        printf("failed to initialize device map\r\n");
        return -1;
    }

    if(NULL == map.get("console")) {
        printf("failed to lookup device 'console'\r\n");
        return -1;
    } else {
        printf("looked up device 'console' successfully\r\n");
    }

    if(!sched_init(&systime)) {
        printf("failed to initialize scheduler\r\n");
        return -1;
    }

    if(-1 == sched_start(&echo)) {
        printf("failed to start echo task\r\n");
        return -1;
    }

    printf("dispatching scheduler...\r\n");

    // main loop
    while(1) {
        sched_dispatch();
        map.poll();
    }
}
