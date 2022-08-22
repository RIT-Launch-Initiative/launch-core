#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <stdint.h>

/// @brief hashmap
/// @tparam T           the type of values in the hashmap
/// @tparam NUM_BUCKETS the number of buckets that can be stored in the map
/// @tparam BUCKET_SIZE the size of each bucket
/// operations are O(BUCKET_SIZE)
template <typename T, const size_t NUM_BUCKETS, const size_t BUCKET_SIZE>
class Hashmap {
public:
    /// @brief constructor
    Hashmap() {
        for(size_t i = 0; i < NUM_BUCKETS * BUCKET_SIZE; i++) {
            m_used[i] = false;
        }
    }

    /// @brief add a value to the map
    /// @param key  the key of the value
    /// @return a pointer to the value to be copied into, or NULL on error
    /// NOTE: the way we add we don't look for collisions with other keys
    /// the first key added will be returned by get until it is removed
    T* add(size_t key) {
        size_t index = (key % NUM_BUCKETS) * BUCKET_SIZE;

        // find an unused location in this bucket
        for(size_t i = index; i < index + BUCKET_SIZE; i++) {
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
    bool rm(size_t key) {
        size_t index = (key % NUM_BUCKETS) * BUCKET_SIZE;

        // search this bucket for an entry with this key
        for(size_t i = index; i < index + BUCKET_SIZE; i++) {
            if(m_used[i]) {
                if(m_entries[i].key == key) {
                    // we found it!
                    m_used[i] = false;
                    return true;
                }
            }
        }

        // oterhwise it's a bad key
        return false;
    }

    /// @brief get the value at a key
    /// @param key  the key of the value
    /// @return a pointer to the value, or NULL on error
    T* get(size_t key) {
        size_t index = (key % NUM_BUCKETS) * BUCKET_SIZE;

        // search this bucket
        for(size_t i = index; i < index + BUCKET_SIZE; i++) {
            if(m_used[i]) {
                if(m_entries[i].key == key) {
                    // we found it!
                    return &(m_entries[i].val);
                }
            }
        }

        return NULL;
    }

private:
    typedef struct {
        size_t key;
        T val;
    } entry_t;

    entry_t m_entries[NUM_BUCKETS * BUCKET_SIZE];
    bool m_used[NUM_BUCKETS * BUCKET_SIZE];
};

#endif
