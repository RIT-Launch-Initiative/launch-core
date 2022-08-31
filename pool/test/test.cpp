#include <stdlib.h>
#include <stdio.h>

#include "pool/pool.h"

bool basic() {
    alloc::Pool<int, 5> pool;

    int* ptrs[5];
    for(size_t i = 0; i < 5; i++) {
        ptrs[i] = pool.alloc();
        if(ptrs[i] == NULL) {
            printf("got NULL on pointer %li\n", i);
            return false;
        }
    }

    if(pool.alloc()) {
        printf("pool did not deny allocating over size limit\n");
        return false;
    }

    for(size_t i = 0; i < 5; i++) {
        if(!pool.free(ptrs[i])) {
            printf("failed to free pointer %lu\n", i);
            return false;
        }
    }

    // allocate one more time to make sure we still can
    for(size_t i = 0; i < 5; i++) {
        ptrs[i] = pool.alloc();
        if(ptrs[i] == NULL) {
            printf("got NULL on pointer %li\n", i);
            return false;
        }
    }
}

bool double_free() {
    alloc::Pool<int, 5> pool;

    int* ptr = pool.alloc();

    if(ptr == NULL) {
        printf("got NULL pointer on alloc\n");
        return false;
    }

    if(!pool.free(ptr)) {
        printf("failed to free ptr\n");
        return false;
    }

    // try and double free
    if(pool.free(ptr)) {
        printf("allowed double free\n");
        return false;
    }

    // see if we can allocated again
    int* ptrs[5];
    for(size_t i = 0; i < 5; i++) {
        ptrs[i] = pool.alloc();
        if(ptrs[i] == NULL) {
            printf("got NULL on pointer %li\n", i);
            return false;
        }
    }

    if(pool.alloc()) {
        printf("pool did not deny allocating over size limit\n");
        return false;
    }

    for(size_t i = 0; i < 5; i++) {
        if(!pool.free(ptrs[i])) {
            printf("failed to free pointer %lu\n", i);
            return false;
        }
    }
}

int main() {
    if(basic()) {
        printf("passed basic test\n");
    } else {
        printf("failed basic test\n");
    }

    if(double_free()) {
        printf("passed double free test\n");
    } else {
        printf("failed double free test\n");
    }
}
