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
    int strlen = 0;
    char string[size] = {};

    String() {}


    /**************************************
     * Element Access
     **************************************/

    int len() {
        return this->strlen;
    }

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
        return this->string[strlen];
    }

    /**
     * Retrieves value at specified index
     * @param index to retrieve element at
     * @return Element at index or nullptr if invalid
     */
    char at(int index) {
        if (index > strlen || index < 0) {
            return NULL_TERMINATOR;
        }

        return this->string[index];
    }

    /**
     * Retrieve a substring of the original string
     *
     * @param start index of substring
     * @param end index of substring
     * @param substr buffer for storing substring
     */
    void substr(int start, int end, char substr[]) {
        if (strlen < start || end < start || strlen < end) return;

        char *substr_head = substr;
        for (int i = start; i < end; i++) {
            *substr = string[i];
            substr++;
        }

        *substr = NULL_TERMINATOR;
        substr = substr_head;
    }

    /**************************************
     * Element Modifiers
     **************************************/

    /**
     * Replaces character at certain position
     * @param position
     * @param character
     */
    void replace(int position, char character) {
        this->string[position] = character;
    }

    /**
     * Replaces a substring of characters
     * @param start
     * @param end
     * @param character
     */
    void replace(int start, int end, char const *character) {
        if (start < 0 || end > size || end < start) return;

        int i;
        for (i = start; i < strlen && i < end; i++) {
            this->string[i] = *character++;
        }

        if (i < end) {
            for (; i < end; i++) {
                this->string[i] = *character++;
                this->strlen++;
            }

            this->string[this->strlen] = NULL_TERMINATOR;
        }
    }

    /**
     * Insert a character at a certain index
     * @param position
     * @param character
     */
    void insert(int position, char character) {
        if (position > strlen || strlen + 1 >= size) return;

        for (int i = strlen + 2; i > position - 1; i--) {
            this->string[i] = this->string[i - 1];
        }

        this->string[position] = character;
        this->strlen++;
    }

    /**
     * Removes character at position specified
     * @param position to remove at
     */
    void remove(int position) {
        if (position > strlen) return;

        for (int i = position; i < strlen + 1; i++) {
            this->string[i] = this->string[i + 1];
        }
        this->strlen--;
    }

    /**
     * Wipes the entire string buffer
     */
    void clear() {
        int i = 0;
        while (this->strlen > 0) {
            string[i++] = NULL_TERMINATOR;
            this->strlen--;
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

        if (len_count != this->strlen) {
            this->strlen = len_count;

            return this->strlen;
        }

        return -1;
    }

    /**************************************
     * Iterators
     **************************************/
    // TODO: Could be nice. Maybe add later after getting base functionality down.

    /**************************************
     * Capacity
     **************************************/

    /**
     * Check if string is empty
     * @return
     */
    bool empty() const {
        return this->strlen == 0;
    }

    /**
     * Check if string fills entire buffer
     * @return
     */
    bool full() const {
        return this->strlen >= size;
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
        return size - strlen - 1;
    }

    /**************************************
     * C API Interfacing
     **************************************/

    /**
     * Checks if there is extra space in the buffer
     * @return boolean on if there's extra space
     */
    bool is_trunc() {
        return this->strlen < size;
    }

    /**
     * Converts remaining characters in buffer to \0
     */
    void init_free_space() {
        // TODO: Use memset or bad for embedded?
        if (is_trunc()) {
            for (int i = strlen; i < size; i++) {
                this->string[i] = NULL_TERMINATOR;
            }
        }
    }

    /**
     * Get the first terminator
     * @return
     */
    char *get_end() {
        return this->string[strlen];
    }

protected:
    String(char const *buffer) {
        int i; // Reduce errors from using strlen as counter

        for (i = 0; i < size - 1; i++) {
            this->string[i] = buffer[i];

            if (this->string[i] == NULL_TERMINATOR) break;
        }

        // Developers should still add a null terminator in their buffers, especially if strlen < buff_size
        if (this->string[i] != NULL_TERMINATOR) {
            this->string[i] = '\0';
        }

        this->strlen = i - 1;
    }


};

namespace str {
    template<const size_t SIZE>
    class String : public ::String<SIZE> {
    public:
        /**
         * String constructor
         * @param buffer
         */
        String(char *buffer) : ::String<SIZE>(buffer) {};

    private:
        char buffer[SIZE];

    };
}


#endif