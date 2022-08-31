#include <stdlib.h>
#include <stdio.h>

#include "vector/vector.h"

bool basic() {
    alloc::Vector<int, 5> vec;

    for(size_t i = 0; i < 5; i++) {
        int* temp = vec.push_back();

        if(temp == NULL) {
            printf("failed to push index: %lu\n", i);
            return false;
        }

        *temp = i;
    }

    if(vec.size() != 5) {
        printf("invalid size\n");
        return false;
    }

    for(size_t i = 0; i < 5; i++) {
        int* temp = vec[i];

        if(*temp != i) {
            printf("invalid data at index: %lu\n", i);
            return false;
        }
    }

    if(RET_SUCCESS != vec.pop_back()) {
        printf("failed to pop back\n");
        return false;
    }

    if(vec.size() != 4) {
        printf("invalid size after pop back\n");
        return false;
    }


    if(RET_SUCCESS != vec.pop_front()) {
        printf("failed to pop front\n");
        return false;
    }

    if(vec.size() != 3) {
        printf("invalid size after pop front\n");
        return false;
    }

    // should now look like [1, 2, 3]

    int* temp = vec.insert(1);
    if(temp == NULL) {
        printf("failed to insert\n");
        return false;
    }

    *temp = 99;

    // should now look like [1, 99, 2, 3]
    temp = vec[0];
    if(*temp != 1) {
        printf("invalid element\n");
        return false;
    }

    temp = vec[1];
    if(*temp != 99) {
        printf("invalid element\n");
        return false;
    }

    temp = vec[2];
    if(*temp != 2) {
        printf("invalid element\n");
        return false;
    }

    temp = vec[3];
    if(*temp != 3) {
        printf("invalid element\n");
        return false;
    }

    return true;
}

int main() {
    if(basic()) {
        printf("basic test passed\n");
    } else {
        printf("basic test failed\n");
    }
}
