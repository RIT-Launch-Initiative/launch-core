#include <stdio.h>

#include "sched/macros/macros.h"

RetType task0(void*) {
    RESUME();

    printf("begin task0 \n");
    volatile int i = 0;

    while(1) {
        printf("task0 - %i\n", i);
        i++;
        YIELD();
    }
}

RetType task1(void*) {
    RESUME();

    printf("begin task1 \n");

    volatile int i = 0;

    while(1) {
        printf("task1 - %i\n", i);
        i++;
        YIELD();
    }
}

unsigned char stack0[4096];
unsigned char stack1[4096];

volatile long int rsp;
volatile long int rbp;
volatile long int rsp0 = (long int)(stack0) + 4096;
volatile long int rbp0 = (long int)(stack0) + 4096;
volatile long int rsp1 = (long int)(stack1) + 4096;
volatile long int rbp1 = (long int)(stack1) + 4096;

int main() {

    while(1) {
        // first dispatch task0
        // switch to it's stack
        sched_dispatched = 0;
        sched_jump[0].index = 0;
        asm("\t mov %%rsp,%0" : "=r"(rsp));
        asm("\t mov %%rbp,%0" : "=r"(rbp));
        asm("\t mov %0,%%rsp" : : "r"(rsp0));
        asm("\t mov %0,%%rbp" : : "r"(rbp0));

        // call it
        task0(NULL);

        asm("\t mov %0,%%rsp" : : "r"(rsp));
        asm("\t mov %0,%%rbp" : : "r"(rbp));

        // now we dispatch task1
        // switch to it's stack
        sched_dispatched = 1;
        sched_jump[1].index = 0;
        asm("\t mov %%rsp,%0" : "=r"(rsp));
        asm("\t mov %%rbp,%0" : "=r"(rbp));
        asm("\t mov %0,%%rsp" : : "r"(rsp1));
        asm("\t mov %0,%%rbp" : : "r"(rbp1));

        // call it
        task1(NULL);

        asm("\t mov %0,%%rsp" : : "r"(rsp));
        asm("\t mov %0,%%rbp" : : "r"(rbp));
    }
}
