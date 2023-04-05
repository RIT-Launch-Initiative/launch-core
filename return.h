#ifndef RETURN_H
#define RETURN_H

// return codes

typedef enum {
    RET_SUCCESS = 0,        // success
    RET_ERROR,              // some error
    RET_YIELD               // not done yet, yield back to the scheduler
} RetType;

#endif
