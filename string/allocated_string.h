//
// Created by aaron on 8/19/22.
//

#ifndef LAUNCH_CORE_ALLOCATED_STRING_H
#define LAUNCH_CORE_ALLOCATED_STRING_H


#include <cstddef>
#include "string.h"

namespace string {
    template <size_t size>
class String : public ::String<size> {
public:
    String(char const *buffer, const size_t str_size) {
    }

};
}


#endif //LAUNCH_CORE_ALLOCATED_STRING_H
