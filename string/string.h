/**
 * Library for embedded strings
 * 
 * @author Aaron Chan
 */

#ifndef STRING_H
#define STRING_H


const char NULL_TERMINATOR = '\0';

// template <const size_t size>
class String {
    public:
        char* string;
        int len;
        size_t size;

        String(char* buffer, size_t buff_size) {
            this->string[buff_size];
            int i;

            for (i = 0; i < buff_size; i++) {
                this->string[i] = buffer[i];
            }

            if (this->string[i] != NULL_TERMINATOR) {
                this->string[i] = '\0';
            }
        }

};


#endif