/*******************************************************************************
*
*  Name: bloom_filter.h
*
*  Purpose: Contains implementation of a width bloom filter.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <stdint.h>

#include "hash/hash.h"

/// @brief bloom filter
///        width of 'size_t' type
/// @tparam T       the object stored in the filter
/// @tparam HASH    the hash to hash the object with, defaults to an XOR hash
template <typename T, typename HASH = XORHash<T>>
class BloomFilter {
public:
    /// @brief constructor
    BloomFilter() : m_data(0), m_hash() {};

    /// @brief encode an object into the filter
    void encode(T& obj) {
        m_data |= m_hash(obj);
    }

    /// @brief check if an object may be present in the filter
    /// @return 'true' if the object MAY be encoded in the filter
    ///         'false' if the object is certainly not in the filter
    bool present(T& obj) {
        return (~m_data & m_hash(obj)) == 0;
    }

private:
    size_t m_data;
    HASH m_hash;
};

#endif
