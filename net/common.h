/*
*   Generic network functions
*/
#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <stdint.h>

#if defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
// big endian to big endian

static inline uint16_t hton16(uint16_t n) {
    return n;
}

static inline uint16_t hton32(uint32_t n) {
    return n;
}

static inline uint16_t hton16(uint64_t n) {
    return n;
}

static inline uint16_t ntoh16(uint16_t n) {
    return n;
}

static inline uint16_t ntoh32(uint32_t n) {
    return n;
}

static inline uint16_t ntoh16(uint64_t n) {
    return n;
}

#else
// little endian to big endian

static inline uint16_t hton16(uint16_t n) {
    uint8_t ret[sizeof(uint16_t)];

    for(int i = sizeof(uint16_t) - 1; i >= 0; i--) {
        ret[i] = n;
        n >>= 8;
    }

    return *((uint16_t*)ret);
}

static inline uint32_t hton32(uint32_t n) {
    uint8_t ret[sizeof(uint32_t)];

    for(int i = sizeof(uint32_t) - 1; i >= 0; i--) {
        ret[i] = n;
        n >>= 8;
    }

    return *((uint32_t*)ret);
}

static inline uint64_t hton64(uint64_t n) {
    uint8_t ret[sizeof(uint64_t)];

    for(int i = sizeof(uint64_t) - 1; i >= 0; i--) {
        ret[i] = n;
        n >>= 8;
    }

    return *((uint64_t*)ret);
}

// big endian to little endian

static inline uint16_t ntoh16(uint16_t n) {
    return hton16(n);
}

static inline uint32_t ntoh32(uint32_t n) {
    return hton32(n);
}

static inline uint64_t ntoh64(uint64_t n) {
    return hton64(n);
}

#endif

#endif
