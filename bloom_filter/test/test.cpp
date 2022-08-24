#include <stdlib.h>
#include <stdio.h>

#include "bloom_filter/bloom_filter.h"

typedef struct {
    uint32_t a;
    uint8_t b;
    int c;
} some_type_t;

bool basic() {
    BloomFilter<some_type_t> filter; // use the default XOR hash

    some_type_t fst = {1, 2, -3};
    filter.encode(fst);

    if(!filter.present(fst)) {
        printf("filter failed to correctly encode first value\n");
        return false;
    }

    some_type_t snd = {4, 5, -6};

    if(filter.present(snd)) {
        printf("second value present when not encoded (could be false positive)\n");
        return false;
    }

    filter.encode(snd);

    if(!filter.present(fst)) {
        printf("first value not present after encoding second value\n");
        return false;
    }

    if(!filter.present(snd)) {
        printf("filter failed to correctly encode second value\n");
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
