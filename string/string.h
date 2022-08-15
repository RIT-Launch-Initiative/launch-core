/**
 * Library for embedded strings
 * 
 * @author Aaron Chan
 */

#ifndef STRING_H
#define STRING_H

#include <cstddef>

const char NULL_TERMINATOR = '\0';

template<const size_t size>
class String {
public:
    int len = 0;
    char string[size] = {};

    /**
     * String constructor
     * @param buffer
     */
    explicit String(char const *buffer) {
        int i; // Reduce errors from using len as counter

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

    /**************************************
     * Element Access
     **************************************/

    /**************************************
     * Element Modifiers
     **************************************/


    /**************************************
     * Iterators
     **************************************/

    /**************************************
     * Capacity
     **************************************/





    /**************************************
     * Improve interfacing with C APIs
     **************************************/

    /**
     * Checks if there is extra space in the buffer
     * @return boolean on if there's extra space
     */
    bool is_trunc() {
        return this->len < size;
    }

    /**
     * Converts remaining characters in buffer to \0
     */
    void init_free_space() {
        if (is_trunc()) {
            for (int i = len; i < size; i++) {
                this->string[i] = NULL_TERMINATOR;
            }
        }
    }

    /**
     * Get the first terminator
     * @return
     */
    char *get_end() {
        return this->string[len];
    }

};


#endif