#ifndef HASH_H
#define HASH_H

#include <stdint.h>

/// @brief hash interface
/// @tparam T   the object type to be hashed
template <typename T>
class Hash {
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
    /// @brief default constructor
    XORHash() {};

    /// @brief return the hash using XORing
    size_t hash(const T& obj) {
        size_t hash = 0;
        const uint8_t* data8 = reinterpret_cast<uint8_t*>(&obj);

        // get to an offset of the output len
        size_t start = sizeof(T) % sizeof(size_t);
        for(size_t i = 0; i < start; i++) {
            hash ^= (data8[i]);
        }

        // hash the rest using the output len size
        const size_t* data = reinterpret_cast<size_t*>(&obj + start);
        for(size_t i = 0; i < (sizeof(T) / sizeof(size_t)); i++) {
            hash ^= data[i];
        }

        return hash;
    }
};

#endif
