#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <stdint.h>

#include "hash/hash.h"

namespace hashmap_internal {

/// @brief hashmap internal entry type
template <typename KEY, typename VALUE>
struct entry_t {
    KEY key;
    VALUE val;
};

};

/// @brief hashmap
/// @tparam KEY     the type of the keys in the hashmap
/// @tparam VALUE   the type of values in the hashmap
/// operations are O(BUCKET_SIZE)
// TODO template the hash function? for now only XORHash is used
template <typename KEY, typename VALUE>
class Hashmap {
public:
    /// @brief add a value to the map
    /// @param key  the key of the value
    /// @return a pointer to the value to be copied into, or NULL on error
    /// NOTE: the way we add we don't look for collisions with other keys
    /// the first key added will be returned by get until it is removed
    VALUE* add(KEY key) {
        size_t index = (key % m_numBuckets) * m_bucketSize;

        // find an unused location in this bucket
        for(size_t i = index; i < index + m_bucketSize; i++) {
            if(!m_used[i]) {
                // this is our location!
                m_entries[i].key = key;
                m_used[i] = true;
                return &(m_entries[i].val);
            }
        }

        return NULL;
    }

    /// @brief remove a value at a key
    /// @param key    the key of the value to remove
    /// @return 'true' if key was deleted, 'false' on error
    bool rm(KEY key) {
        size_t index = (m_hash.hash(key) % m_numBuckets) * m_bucketSize;

        // search this bucket for an entry with this key
        for(size_t i = index; i < index + m_bucketSize; i++) {
            if(m_used[i]) {
                if(m_entries[i].key == key) {
                    // we found it!
                    m_used[i] = false;
                    return true;
                }
            }
        }

        // otherwise it's a bad key
        return false;
    }

    /// @brief get the value at a key
    /// @param key  the key of the value
    /// @return a pointer to the value, or NULL on error
    VALUE* get(KEY key) {
        size_t index = (m_hash.hash(key) % m_numBuckets) * m_bucketSize;

        // search this bucket
        for(size_t i = index; i < index + m_bucketSize; i++) {
            if(m_used[i]) {
                if(m_entries[i].key == key) {
                    // we found it!
                    return &(m_entries[i].val);
                }
            }
        }

        return NULL;
    }

    /// @brief get the value at a key
    /// @param key  the key of the value
    /// @return a pointer to the value, or NULL on error
    inline VALUE* operator[](KEY key) {
        return get(key);
    }

protected:
    /// @brief protected constructor, use alloc::Hashmap to declare instead
    Hashmap(size_t num_buckets, size_t bucket_size,
            hashmap_internal::entry_t<KEY, VALUE>* entries,
            bool* used, Hash<KEY>& hash) :
                                            m_numBuckets(num_buckets),
                                            m_bucketSize(bucket_size),
                                            m_entries(entries),
                                            m_used(used),
                                            m_hash(hash) {
        for(size_t i = 0; i < num_buckets * bucket_size; i++) {
            m_used[i] = false;
        }
    }

private:
    // size = num buckets X bucket size
    hashmap_internal::entry_t<KEY, VALUE>* m_entries;
    bool* m_used;

    size_t m_numBuckets;
    size_t m_bucketSize;

    Hash<KEY>& m_hash;
};

namespace alloc {

/// @brief preallocated hashmap
/// @tparam KEY           the type of keys in the hashmap
/// @tparam value         the type of values in the hashmap
/// @tparam NUM_BUCKETS   the number of buckets that can be stored in the map
/// @tparam BUCKET_SIZE   the size of each bucket
/// @tparam HASH          the hash to use for keys, defaults to basic XOR hash
/// operations are O(BUCKET_SIZE)
template <typename KEY, typename VALUE,
          const size_t NUM_BUCKETS, const size_t BUCKET_SIZE,
          typename HASH = XORHash<KEY>>
class Hashmap : public ::Hashmap<KEY, VALUE> {
public:
    /// @brief constructor
    Hashmap() : ::Hashmap<KEY, VALUE>(NUM_BUCKETS,
                                      BUCKET_SIZE,
                                      m_internalEntries,
                                      m_internalUsed,
                                      m_internalHash) {};

private:
    hashmap_internal::entry_t<KEY, VALUE> m_internalEntries[NUM_BUCKETS * BUCKET_SIZE];
    bool m_internalUsed[NUM_BUCKETS * BUCKET_SIZE];
    HASH m_internalHash;
};

}

#endif
