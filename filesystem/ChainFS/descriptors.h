/*******************************************************************************
*
*  Name: descriptors.h
*
*  Purpose: Describes descriptors (heh)
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <stdint.h>

namespace chainfs {

// maximum file name length, including NULL terminator
static const size_t MAX_FILENAME_SIZE = 16;

/// @brief type of each descriptor
#define NAME_DESCRIPTOR 0x01               // maps a name to a file index
#define DATA_DESCRIPTOR 0x02               // describes file data
#define FREE_DESCRIPTOR 0x03               // describes a chunk of free blocks

/// @brief descriptor
typedef struct {
    uint32_t type;              // type of the descriptor
    uint32_t next_block;        // starting block of the next descriptor, or 0xFFFFFFFF if this is the last descriptor
    // uint32_t next_offset;       // offset of the next descriptor
} header_t;

/// @brief name descriptor
typedef struct {
    header_t header;
    uint32_t data;                  // block number of first data descriptor
    char name[MAX_FILENAME_SIZE];   // filename (NULL terminated)
} name_descriptor_t;

/// @brief data descriptor
typedef struct {
    header_t header;
    uint32_t start;         // starting block
    uint32_t allocated;     // number of blocks allocated for this data
    uint32_t used;          // number of blocks used for file data
    uint32_t offset;        // number of bytes written in the last block
    uint32_t next_block;    // block number where the next data descriptor lies
} data_descriptor_t;

/// @brief free desciptor
typedef struct {
    header_t header;
    uint32_t start;         // first block that's free
    uint32_t len;           // number of blocks that are free after start
} free_descriptor_t;

typedef union {
    header_t header_only;
    name_descriptor_t name_desc;
    data_descriptor_t data_desc;
    free_descriptor_t free_desc;
} descriptor_t;

} // namespace chainfs


#endif
