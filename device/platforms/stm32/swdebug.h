//
// Created by Yevgeniy Gorbachev on 2023-06-03.
//

#ifndef SENSOR_MODULE_SWDEBUG_H
#define SENSOR_MODULE_SWDEBUG_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

void swprint(const char* msg);
int swprintf(const char* fmt, ...);
void swprintx(const char* leader, const uint8_t* bytes, size_t len);
extern uint32_t ITM_SendChar(uint32_t ch);

#endif //SENSOR_MODULE_SWDEBUG_H
