#ifndef SENSOR_MODULE_SWDEBUG_H
#define SENSOR_MODULE_SWDEBUG_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

void swprint(const char* msg);
int swprintf(const char* fmt, ...);
void swprintx(const char* leader, const uint8_t* bytes, size_t len);

#include "core_cm4.h"

/// @brief send a plain string using ITM_SendChar if DEBUG is defined
void swprint(const char* msg) {
#ifdef DEBUG
    int len = strlen(msg);
    for (int i = 0; i < len; i++) {
        ITM_SendChar(msg[i]);
    }
#endif
}

/// @brief send a formatted string using ITM_SendChar if DEBUG is defined
/// (NOTE: fails if the resulting message is more than 256 bytes)
/// @return The return value of vsnprintf (0 if DEBUG is not defined)
int swprintf(const char* fmt, ...) {
#ifdef DEBUG
    va_list ap;
    va_start(ap, fmt);
    char msg[256];
    int status = vsnprintf(msg, 256, fmt, ap);
    va_end(ap);

    if (status > 0) {
        for (int i = 0; i < status; i++) {
            ITM_SendChar(msg[i]);
        }
    }

    return status;
#else
    return 0;
#endif
}

/// @brief send a byte array as hex separated by spaces
void swprintx(const char* leader, const uint8_t* bytes, size_t len) {
#ifdef DEBUG
    swprint(leader);
    for (int i = 0; i < len; i++) {
        swprintf("%02X ", bytes[i]);
    }
    swprint("\n");
#endif
}
#endif //SENSOR_MODULE_SWDEBUG_H
