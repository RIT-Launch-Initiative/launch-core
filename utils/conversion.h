/**
 * Defines helper functions for converting between different integer types
 * 
 * @author Aaron Chan
 */
#ifndef LAUNCH_CORE_CONVERSION_H
#define LAUNCH_CORE_CONVERSION_H

#define uint16_to_uint8(x, buff) \
    *(buff) = (x >> 8) & 0xFF;   \
    *(buff + 1) = x & 0xFF;

#define uint32_to_uint8(x, buff) \
    *(buff) = (x >> 24) & 0xFF;  \
    *(buff + 1) = (x >> 16) & 0xFF; \
    *(buff + 2) = (x >> 8) & 0xFF;  \
    *(buff + 3) = x & 0xFF;



#define uint64_to_uint8(x, buff) \
    *(buff) = (x >> 56) & 0xFF;  \
    *(buff + 1) = (x >> 48) & 0xFF; \
    *(buff + 2) = (x >> 40) & 0xFF; \
    *(buff + 3) = (x >> 32) & 0xFF; \
    *(buff + 4) = (x >> 24) & 0xFF; \
    *(buff + 5) = (x >> 16) & 0xFF; \
    *(buff + 6) = (x >> 8) & 0xFF;  \
    *(buff + 7) = x & 0xFF;

#define int16_to_uint8(x, buff) \
    *(buff) = (x >> 8) & 0xFF;  \
    *(buff + 1) = x & 0xFF;

#define int32_to_uint8(x, buff) \
    *(buff) = (x >> 24) & 0xFF; \
    *(buff + 1) = (x >> 16) & 0xFF; \
    *(buff + 2) = (x >> 8) & 0xFF;  \
    *(buff + 3) = x & 0xFF;

#define int64_to_uint8(x, buff) \
    *(buff) = (x >> 56) & 0xFF;  \
    *(buff + 1) = (x >> 48) & 0xFF; \
    *(buff + 2) = (x >> 40) & 0xFF; \
    *(buff + 3) = (x >> 32) & 0xFF; \
    *(buff + 4) = (x >> 24) & 0xFF; \
    *(buff + 5) = (x >> 16) & 0xFF; \
    *(buff + 6) = (x >> 8) & 0xFF;  \
    *(buff + 7) = x & 0xFF;

#define uint8_to_uint16(buff) \
    ((uint16_t) *buff << 8) | *(buff + 1)

#define uint8_to_uint32(buff) \
    ((uint32_t) *buff << 24) | ((uint32_t) *(buff + 1) << 16) | ((uint32_t) *(buff + 2) << 8) | *(buff + 3)

#define uint8_to_uint64(buff) \
    ((uint64_t) *buff << 56) | ((uint64_t) *(buff + 1) << 48) | ((uint64_t) *(buff + 2) << 40) | ((uint64_t) *(buff + 3) << 32) | \
    ((uint64_t) *(buff + 4) << 24) | ((uint64_t) *(buff + 5) << 16) | ((uint64_t) *(buff + 6) << 8) | *(buff + 7)

#define uint8_to_int16(buff) \
    ((int16_t) *(buff) << 8) | *(buff + 1)

#define uint8_to_int32(buff) \
    ((int32_t) *buff << 24) | ((int32_t) *(buff + 1) << 16) | ((int32_t) *(buff + 2) << 8) | *(buff + 3)


#define uint8_to_int64(buff) \
    ((int64_t) *buff << 56) | ((int64_t) *(buff + 1) << 48) | ((int64_t) *(buff + 2) << 40) | ((int64_t) *(buff + 3) << 32) | \
    ((int64_t) *(buff + 4) << 24) | ((int64_t) *(buff + 5) << 16) | ((int64_t) *(buff + 6) << 8) | *(buff + 7)


#endif //LAUNCH_CORE_CONVERSION_H
