#include <stdlib.h>
#include <stdio.h>

#include "hashmap/hashmap.h"

bool basic() {
    Hashmap<int, 5, 1> hm;

    int* val = hm.add(1);
    *val = 10;

    val = hm.add(7);
    *val = 12;

    val = hm.get(1);
    if(val == NULL) {
        printf("get returned NULL (1)\n");
        return false;
    }

    if(*val != 10) {
        printf("get returning incorrect value\n");
        return false;
    }

    if(!hm.rm(1)) {
        printf("rm failed\n");
        return false;
    }

    val = hm.get(7);
    if(val == NULL) {
        printf("get returned NULL (7)\n");
        return false;
    }

    if(*val != 12) {
        printf("get returning incorrect value\n");
        return false;
    }
}

int main() {
    if(basic()) {
        printf("basic test passed\n");
    } else {
        printf("basic test failed\n");
    }
}
