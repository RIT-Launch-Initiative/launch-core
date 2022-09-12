/*******************************************************************************
*
*  Name: hash.h
*
*  Purpose: Contains definition for a hash object and some common hashes.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef HASH_H
#define HASH_H

#include <stdint.h>

/// @brief hash interface
/// @tparam T   the object type to be hashed
template <typename T>
class Hash {
public:
    /// @brief default constructor
    Hash() {};

    /// @brief calculate the hash
    /// @return the hash
    virtual size_t hash(const T& obj) = 0;

    /// @brief calculate the hash
    /// @return the hash
    inline size_t operator()(const T& obj) {
        return hash(obj);
    }
};

/// @brief hashes an object using a basic XOR
/// @tparam T   the object type to be hashed
template <typename T>
class XORHash : public Hash<T> {
public:
    /// @brief default constructor
    XORHash() {};

    /// @brief return the hash using XORing
    size_t hash(const T& obj) {
        size_t hash = 0;
        const uint8_t* data8 = reinterpret_cast<const uint8_t*>(&obj);

        // get to an offset of the output len
        size_t start = sizeof(T) % sizeof(size_t);
        for(size_t i = 0; i < start; i++) {
            hash ^= (data8[i]);
        }

        // hash the rest using the output len size
        const size_t* data = reinterpret_cast<const size_t*>(&obj + start);
        for(size_t i = 0; i < (sizeof(T) / sizeof(size_t)); i++) {
            hash ^= data[i];
        }

        return hash;
    }
};

/// @brief hash a C-style string
class StringHash : public Hash<char*> {
    /// @brief constructor
    StringHash();

    /// @brief return a hash of the string
    /// @return the hash
    size_t hash(const char*& obj) {
        size_t hash = 0;
        size_t i = 0;
        size_t ii = 0;

        while(obj[i]) {
            hash ^= (((size_t)obj[i]) << (ii * 8));
            i++;

            ii++;
            i %= sizeof(hash);
        }

        return hash;
    }
};

#endif
