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

        this->len = i - 1;
    }

    /**************************************
     * Element Access
     **************************************/
     char* front() {
         return this->string[0];
     }

     char* back() {
         return this->string[len - 1];
     }

     char* at(int index) {
         if (index > len) return nullptr;

         return this->string[index];
     }

     char* substr(int start, int end) {
         if (len < start || end < start || len < end) return nullptr;

         char sub[(end - start) + 1];
         int sub_index = 0;

         for (int i = start; i < end; i++) {
             sub[sub_index++] = string[i];
         }

         sub[sub_index] = NULL_TERMINATOR;

         return sub; // TODO: Change this. This will go out of scope.
     }

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