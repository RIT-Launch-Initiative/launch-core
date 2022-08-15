/**
 * Library for embedded strings
 * 
 * @author Aaron Chan
 */

#ifndef STRING_H
#define STRING_H

#include <cstddef>

const char NULL_TERMINATOR = '\0';

template <const size_t size>
class String {
    public:
        int len;
        char string[size];

        String(char const* buffer): len(0), string({}) {
            int i;

            for (i = 0; i < size - 1; i++) {
                this->string[i] = buffer[i];

                if (this->string[i] == NULL_TERMINATOR) break;
            }

            // Developers should still add a null terminator in their buffers, especially if len < buff_size
            if (this->string[i] != NULL_TERMINATOR) {


                this->string[i] = '\0';
            }

            this->len = i;
        }

};


#endif