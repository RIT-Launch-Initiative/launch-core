#include <stdio.h>


typedef struct {
    long int rsp;
    void* ra;
    unsigned char stack[4096];
} tcb_t;


tcb_t task1;
tcb_t task2;

void dispatch();

void task1_func() {
    task1.rsp = (long int)(&(task1.stack)) + 4096;

    asm("\t mov %0,%%rsp" : : "r"(task1.rsp));
    asm("\t mov %0,%%rbp" : : "r"((long int)(&(task1.stack))));

    while(1) {
        printf("task1\n");
        dispatch();
    }
}

void task2_func() {
    task2.rsp = (long int)(&(task2.stack)) + 4096;

    asm("\t mov %0,%%rsp" : : "r"(task2.rsp));
    asm("\t mov %0,%%rbp" : : "r"((long int)(&(task2.stack))));

    while(1) {
        printf("task2\n");
        dispatch();
    }
}


int i = 2;
void dispatch() {
    if(i == 2) {
        i = 3;
        task1_func();

        printf("task 1 unexpectedly returned\n");
        while(1) {};
    }
    if(i == 3) {
        i = 1;
        task2_func();

        printf("task 2 unexpectedly returned\n");
        while(1) {};
    }

    i += 1;
    i %= 2;

    if(i == 0) {
        // save task 2
        // stack pointer
        asm("\t mov %%rsp,%0" : "=rm"(task2.rsp));

        // return address
        task2.ra = __builtin_return_address(0);

       // now swap stacks to task 1
       asm("\t mov %0,%%rsp" : : "r"(task1.rsp));
       asm("\t mov %0,%%rbp" : : "r"((long int)(&(task1.stack))));

        // now jump to task 1's return address from when it last called dispatch
        goto *(task1.ra);
       return;
    } else {
        // save task 1
        // stack pointer
        asm("\t mov %%rsp,%0" : "=rm"(task1.rsp));

        // return address
        task1.ra = __builtin_return_address(0);

       // now swap stacks to task 2
       asm("\t mov %0,%%rsp" : : "r"(task2.rsp));
       asm("\t mov %0,%%rbp" : : "r"((long int)(&(task2.stack))));

        // now jump to task 2's return address from when it last called dispatch
        goto *(task2.ra);
    }
}

int main() {
    long int i;
    long int j;
    asm("\t mov %%rsp,%0" : "=rm"(i));
    asm("\t mov %%rbp,%0" : "=rm"(i));

    printf("rsp = %p\n", (void*)i);
    printf("rbp = %p\n", (void*)j);

    dispatch();
}
