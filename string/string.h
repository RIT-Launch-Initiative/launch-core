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

    /**
     * Retrieves first value in string
     * @return First Element in String
     */
    char front() {
        return this->string[0];
    }

    /**
     * Retrieves last value in string
     * @return Last Element in String
     */
    char back() {
        return this->string[len];
    }

    /**
     * Retrieves value at specified index
     * @param index to retrieve element at
     * @return Element at index or nullptr if invalid
     */
    char at(int index) {
        if (index > len || index < 0) {
            return NULL_TERMINATOR;
        }

        return this->string[index]; // TODO: Not actually returning character
    }

    /**
     * Retrieve a substring of the original string
     *
     * @param start index of substring
     * @param end index of substring
     * @return Substring of String
     */
    char *substr(int start, int end) {
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
    void replace(int position, char character) {
        this->string[position] = character;
    }

    void replace(int start, int end, char const *character) {
        if (start < 0 || end > size || end < start) return;

        int i;
        for (i = start; i < len; i++) {
            this->string[i] = *character++;
        }

        if (i < end) {
            for (; i < end; i++) {
                this->string[i] = *character++;
                this->len++;
            }

            this->string[++i] = NULL_TERMINATOR;
        }
    }

    void insert(int position, char character) {
        if (position > len || len + 1 >= size) return;

        for (int i = len + 2; i > position - 1; i--) {
            this->string[i] = this->string[i - 1];
        }

        this->string[position] = character;
        this->len++;
    }

    /**
     * Removes character at position specified
     * @param position to remove at
     */
    void remove(int position) {
        if (position > len) return;

        for (int i = position; i < len + 1; i++) {
            this->string[i] = this->string[i + 1];
        }
        this->len--;
    }

    /**
     * Wipes the entire string buffer
     */
    void clear() {
        int i = 0;
        while (this->len > 0) {
            string[i++] = NULL_TERMINATOR;
            this->len--;
        }
    }

    /**
     * Recount the length of the string in case
     * null gets randomly added to a string
     *
     * @return New Length or -1 if unchanged
     */
    int recount() {
        int len_count = 0;

        while (this->string[(len_count++) + 1] != NULL_TERMINATOR);

        if (len_count != this->len) {
            this->len = len_count;

            return this->len;
        }

        return -1;
    }

    /**************************************
     * Iterators
     **************************************/

    /**************************************
     * Capacity
     **************************************/

    /**
     * Check if string is empty
     * @return
     */
    bool empty() const {
        return this->len == 0;
    }

    /**
     * Check if string fills entire buffer
     * @return
     */
    bool full() const {
        return this->len >= size;
    }

    /**
     * Get the max size of the buffer
     * @return
     */
    size_t max_size() const {
        return size;
    }

    /**
     * Get the remaining available space in the buffer
     * @return
     */
    size_t available() const {
        return size - len - 1;
    }

    /**************************************
     * C API Interfacing
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
        // TODO: Use memset or bad for embedded?
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