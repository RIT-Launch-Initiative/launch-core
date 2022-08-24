#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "ringbuffer/RingBuffer.h"

uint8_t buff[3];
alloc::RingBuffer<3, 1> rb;

uint8_t out[3];
uint8_t test[3];
uint8_t dummy[3];

int main() {
    test[0] = 1;
    test[1] = 2;
    test[2] = 3;

    dummy[0] = 54;
    dummy[1] = 55;
    dummy[2] = 56;

    rb.push(test, 3);
    rb.pop(out, 3);
    printf("%i, %i, %i\n", out[0], out[1], out[2]); // {1, 2, 3}

    rb.push(test, 3);
    rb.push(dummy, 1);
    rb.pop(out, 3);
    printf("%i, %i, %i\n", out[0], out[1], out[2]); // {2,3,54}

    rb.push(test, 3);
    rb.push(dummy, 1);
    rb.push(&dummy[1], 1);
    rb.pop(out, 3);
    printf("%i, %i, %i\n", out[0], out[1], out[2]); // {3,54,55}

    rb.push(dummy, 3);
    rb.pop(out, 3);
    printf("%i, %i, %i\n", out[0], out[1], out[2]); // {54,55,56}
}
