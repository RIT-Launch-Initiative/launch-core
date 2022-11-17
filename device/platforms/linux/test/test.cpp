#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "device/platforms/linux/test/map.h"
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

    printf("echo task started\r\n");

    Device* dev = map.get("console");
    if(dev == NULL) {
        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

    static StreamDevice* console = reinterpret_cast<StreamDevice*>(dev);

    if(RET_SUCCESS != console->obtain()) {
        printf("failed to acquire console device\r\n");

        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

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

        // yield our time back to the scheduler
        YIELD();
    }

    // should never get here
    RESET();
    console->release();
    return RET_ERROR;
}

// basic timer task
RetType timer() {
    RESUME();

    printf("timer task started\r\n");

    Device* dev = map.get("debug");
    if(dev == NULL) {
        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

    static StreamDevice* debug = reinterpret_cast<StreamDevice*>(dev);

    if(RET_SUCCESS != debug->obtain()) {
        printf("failed to acquire debug device\r\n");

        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

    static uint8_t msg[8] = "ding!\r\n";

    while(1) {
        SLEEP(1000);
        debug->write(msg, 7);
    }

    // should never get here
    RESET();
    debug->release();
    return RET_ERROR;
}

// another basic timer task
RetType timer2() {
    RESUME();

    printf("timer2 task started\r\n");

    Device* dev = map.get("debug");
    if(dev == NULL) {
        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

    static StreamDevice* debug = reinterpret_cast<StreamDevice*>(dev);

    if(RET_SUCCESS != debug->obtain()) {
        printf("failed to acquire debug device\r\n");

        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

    static uint8_t msg[8] = "dong!\r\n";

    while(1) {
        SLEEP(1000);
        debug->write(msg, 7);
    }

    // should never get here
    RESET();
    debug->release();
    return RET_ERROR;
}

// slow timer task
RetType slow_timer() {
    RESUME();

    printf("slow_timer task started\r\n");

    Device* dev = map.get("debug");
    if(dev == NULL) {
        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

    static StreamDevice* debug = reinterpret_cast<StreamDevice*>(dev);

    if(RET_SUCCESS != debug->obtain()) {
        printf("failed to acquire debug device\r\n");

        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

    static uint8_t msg[8] = "dung?\r\n";

    while(1) {
        SLEEP(5000);
        debug->write(msg, 7);
    }

    // should never get here
    RESET();
    debug->release();
    return RET_ERROR;
}

// task that waits to receive packets on the network
RetType echo_net() {
    RESUME();

    printf("network echo task started\r\n");

    Device* dev = map.get("socket_pool");
    if(dev == NULL) {
        printf("failed to get socket pool device\r\n");
        // returning error removes the task from the scheduler
        return RET_ERROR;
    }

    SocketPool* pool = reinterpret_cast<SocketPool*>(dev);

    static Socket* sock = pool->alloc();
    if(NULL == sock) {
        printf("failed to allocate socket from socket pool\r\n");
        return RET_ERROR;
    }

    sockaddr_t addr;
    addr.port = 8000;
    addr.addr = 0; // listen to any interface

    if(RET_SUCCESS != sock->bind(&addr)) {
        printf("failed to bind socket to addr\r\n");
        return RET_ERROR;
    }

    while(1) {
        if(sock->available()) {
            printf("UDP packet received!\r\n");

            // get off the scheduler
            printf("exiting network echo task\r\n");
            return RET_ERROR;
        }

        YIELD();
    }
}

uint32_t tick = 0;
uint32_t systime() {
    usleep(1000);

    tick++;
    return tick;
}

int main() {
    set_conio_terminal_mode();

    // TODO put in init task? that does init work and spins or flashes an LED or something?
    if(RET_SUCCESS != map.init()) {
        printf("failed to initialize device map\r\n");
        return -1;
    }

    #ifdef DEBUG
    map.print();
    #endif

    if(NULL == map.get("console")) {
        printf("failed to lookup device 'console'\r\n");
        return -1;
    } else {
        printf("looked up device 'console' successfully\r\n");
    }

    if(NULL == map.get("debug")) {
        printf("failed to lookup device 'debug'\r\n");
        return -1;
    } else {
        printf("looked up device 'debug' successfully\r\n");
    }

    if(!sched_init(&systime)) {
        printf("failed to initialize scheduler\r\n");
        return -1;
    }

    if(-1 == sched_start(&echo)) {
        printf("failed to start echo task\r\n");
        return -1;
    }

    if(-1 == sched_start(&timer)) {
        printf("failed to start timer task\r\n");
        return -1;
    }

    if(-1 == sched_start(&timer2)) {
        printf("failed to start timer2 task\r\n");
        return -1;
    }

    if(-1 == sched_start(&slow_timer)) {
        printf("failed to start slow_timer task\r\n");
        return -1;
    }

    if(-1 == sched_start(&echo_net)) {
        printf("failed to start network echo task\r\n");
        return -1;
    }

    printf("dispatching scheduler...\r\n");

    // main.cpp loop
    while(1) {
        sched_dispatch();
        map.poll();
    }
}
