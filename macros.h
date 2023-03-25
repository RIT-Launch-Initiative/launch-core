#ifndef MACROS_H
#define MACROS_H

#include <cstddef>

// some common macros

// same as Linux kernel container_of
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


// spins forever, executing expression EXP
// good for terminatiing functions that should never exit
#define SPIN_TO_WIN(EXP) while(1) {EXP;}


// branch prediction hints (GCC only)
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#endif
