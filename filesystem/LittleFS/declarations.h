//
// Created by aaron on 11/19/22.
//

#ifndef LAUNCH_CORE_LFS_DECLARATIONS_H
#define LAUNCH_CORE_LFS_DECLARATIONS_H

namespace lfs {
    static const size_t MAX_BLOCK_SIZE = 2048;
    static const MAX_NAME_SIZE = 255;
    static const MAX_FILE_SIZE = 2147483647; // TODO: Determine file size or make it configurable
    static const MAX_ATTRIBUTES = 1022;

    typedef uint32_t LFS_SIZE_T;
    typedef uint32_t LFS_OFFSET_T;
    typedef int32_t LFS_SSIZE_T;
    typedef int32_t LFS_SOFFSET_T;
    typedef uint32_t LFS_BLOCK_T;

    typedef enum {
        LFS_RET_OK = 0,    // No error
        LFS_RET_IO = -5,   // Error during device operation
        LFS_RET_CORRUPT = -84,  // Corrupted
        LFS_RET_NOENT = -2,   // No directory entry
        LFS_RET_EXIST = -17,  // Entry already exists
        LFS_RET_NOTDIR = -20,  // Entry is not a dir
        LFS_RET_ISDIR = -21,  // Entry is a dir
        LFS_RET_NOTEMPTY = -39,  // Dir is not empty
        LFS_RET_BADF = -9,   // Bad file number
        LFS_RET_FBIG = -27,  // File too large
        LFS_RET_INVAL = -22,  // Invalid parameter
        LFS_RET_NOSPC = -28,  // No space left on device
        LFS_RET_NOMEM = -12,  // No more memory available
        LFS_RET_NOATTR = -61,  // No data/attr available
        LFS_RET_NAMETOOLONG = -36,  // File name too long
    } LFS_RET;

    typedef enum {
        // file types
        LFS_TYPE_REG = 0x001,
        LFS_TYPE_DIR = 0x002,

        // internally used types
        LFS_TYPE_SPLICE = 0x400,
        LFS_TYPE_NAME = 0x000,
        LFS_TYPE_STRUCT = 0x200,
        LFS_TYPE_USERATTR = 0x300,
        LFS_TYPE_FROM = 0x100,
        LFS_TYPE_TAIL = 0x600,
        LFS_TYPE_GLOBALS = 0x700,
        LFS_TYPE_CRC = 0x500,

        // internally used type specializations
        LFS_TYPE_CREATE = 0x401,
        LFS_TYPE_DELETE = 0x4ff,
        LFS_TYPE_SUPERBLOCK = 0x0ff,
        LFS_TYPE_DIRSTRUCT = 0x200,
        LFS_TYPE_CTZSTRUCT = 0x202,
        LFS_TYPE_INLINESTRUCT = 0x201,
        LFS_TYPE_SOFTTAIL = 0x600,
        LFS_TYPE_HARDTAIL = 0x601,
        LFS_TYPE_MOVESTATE = 0x7ff,

        // internal chip sources
        LFS_FROM_NOOP = 0x000,
        LFS_FROM_MOVE = 0x101,
        LFS_FROM_USERATTRS = 0x102,
    } LFS_TYPE;

    typedef struct {
        LFS_TYPE type;
        LFS_SIZE size;
        char name[MAX_NAME_SIZE + 1];
    } LFS_INFO;

    typedef struct {
        uint8_t type;
        void *buffer;
        LFS_SIZE_T size;
    };

    typedef enum {
        // open flags
        LFS_O_RDONLY = 1,         // Open a file as read only
        LFS_O_WRONLY = 2,         // Open a file as write only
        LFS_O_RDWR = 3,         // Open a file as read and write
        LFS_O_CREAT = 0x0100,    // Create a file if it does not exist
        LFS_O_EXCL = 0x0200,    // Fail if a file already exists
        LFS_O_TRUNC = 0x0400,    // Truncate the existing file to zero size
        LFS_O_APPEND = 0x0800,    // Move to end of file on every write

        // internally used flags
        LFS_F_DIRTY = 0x010000, // File does not match storage
        LFS_F_WRITING = 0x020000, // File has been written since last flush
        LFS_F_READING = 0x040000, // File has been read since last flush
        LFS_F_ERRED = 0x080000, // An error occurred during write
        LFS_F_INLINE = 0x100000, // Currently inlined in directory entry
    } LFS_FLAGS;

    typedef enum {
        LFS_SEEK_SET = 0,
        LFS_SEEK_CUR = 1,
        LFS_SEEK_END = 2,
    } LFS_SEEK_FLAGS;

    // TODO: LFS Config

    /***************************************************
     *               DATA STRUCTURES                   *
     ***************************************************/
    typedef struct {
        LFS_BLOCK_T block;
        LFS_OFFSET_T offset;
        LFS_SIZE_T size;
        uint8_t *buffer;
    } LFS_CACHE_T;

    typedef struct {
        LFS_BLOCK_T pair[2];
        uint32_t rev;
        LFS_OFFSET_T off;
        uint32_t etag;
        uint16_t count;
        bool erased;
        bool split;
        LFS_BLOCK_T tail[2];
    } LFS_MDIR_T;

    // littlefs directory type
    typedef struct LFS_DIR {
        struct LFS_DIR *next;
        uint16_t id;
        uint8_t type;
        lfs_mdir_t m;

        lfs_off_t pos;
        lfs_block_t head[2];
    } LFS_DIR_T;

// littlefs file type
    typedef struct LFS_FILE {
        struct LFS_FILE *next;
        uint16_t id;
        uint8_t type;
        lfs_mdir_t m;

        struct lfs_ctz {
            lfs_block_t head;
            lfs_size_t size;
        } ctz;

        uint32_t flags;
        lfs_off_t pos;
        lfs_block_t block;
        lfs_off_t off;
        lfs_cache_t cache;

        const struct LFS_FILE_CONFIG *cfg;
    } LFS_FILE_T;

    typedef struct LFS_SUPER_BLOCK {
        uint32_t version;
        lfs_size_t block_size;
        lfs_size_t block_count;
        lfs_size_t name_max;
        lfs_size_t file_max;
        lfs_size_t attr_max;
    } LFS_SUPER_BLOCK_T;

    typedef struct LFS_GSTATE {
        uint32_t tag;
        lfs_block_t pair[2];
    } LFS_GSTATE_T;



#endif //LAUNCH_CORE_LFS_DECLARATIONS_H
