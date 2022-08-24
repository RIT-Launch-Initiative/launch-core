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

int main() {
    if(basic()) {
        printf("passed basic test\n");
    } else {
        printf("failed basic test\n");
    }
}
