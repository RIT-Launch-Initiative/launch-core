/**
 * Library for embedded strings
 *
 * @author Aaron Chan
 */

#ifndef STRING_H
#define STRING_H

const char NULL_TERMINATOR = '\0';

class String {
public:
    char *string;
    int strlen = 0;
    int size;

    /**************************************
     * Element Access
     **************************************/

    /**
     * Get length of string
     *
     * @return String Length
     */
    int len() const {
        return this->strlen;
    }

    /**
     * Retrieves first value in string
     * @return First Element in String
     */
    char front() const {
        return this->string[0];
    }

    /**
     * Retrieves last value in string
     * @return Last Element in String
     */
    char back() const {
        return this->string[strlen];
    }

    /**
     * Retrieves value at specified index
     * @param index to retrieve element at
     * @return Element at index or nullptr if invalid
     */
    char at(int index) const {
        if (index > strlen || index < 0) {
            return NULL_TERMINATOR;
        }

        return this->string[index];
    }

    char* at_ptr(int index) const {
        if (index > strlen || index < 0) {
            return nullptr;
        }

        return &this->string[index];
    }

    /**
     * Retrieve a substring of the original string
     *
     * @param start index of substring
     * @param end index of substring
     * @param substr buffer for storing substring
     */
    void substr(int start, int end, char substr[]) const {
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
     * Checks to see if the string buffer is full
     *
     * @return boolean on if there buffer is filled
     */
    bool is_full() const {
        return this->strlen >= size;
    }

    /**
     * Checks if there is extra space in the buffer
     * @return boolean on if there's extra space
     */
    bool is_trunc() const {
        return this->strlen < size;
    }

    /**
     * Get the first terminator
     * @return
     */
    char *get_end() const {
        return this->string + len();
    }

protected:
    String(char *buffer, size_t size): size(size), string(buffer) {}
};

namespace str {
    template<const size_t SIZE>
    class String : public ::String {
    public:
        /**
         * String constructor
         * @param buffer
         */
        explicit String(char *buffer) : ::String(buffer, SIZE) {};

    };
}


#endif