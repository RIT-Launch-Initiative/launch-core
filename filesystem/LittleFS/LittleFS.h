//
// Created by aaron on 3/14/23.
//

#ifndef LAUNCH_CORE_LITTLEFS_H
#define LAUNCH_CORE_LITTLEFS_H

#include "filesystem/FileSystem.h"
#include "device/BlockDevice.h"
#include "lfs.h"
#include "lfs_util.h"
#include "sched/macros.h"

#define LFS_MKATTRS(...) \
    (struct lfs_mattr[]){__VA_ARGS__}, \
    sizeof((struct lfs_mattr[]){__VA_ARGS__}) / sizeof(struct lfs_mattr)

#define LFS_MKTAG(type, id, size) \
    (((lfs_tag_t)(type) << 20) | ((lfs_tag_t)(id) << 10) | (lfs_tag_t)(size))

#define LFS_MKTAG_IF(cond, type, id, size) \
    ((cond) ? LFS_MKTAG(type, id, size) : LFS_MKTAG(LFS_FROM_NOOP, 0, 0))

#define LFS_MKTAG_IF_ELSE(cond, type1, id1, size1, type2, id2, size2) \
    ((cond) ? LFS_MKTAG(type1, id1, size1) : LFS_MKTAG(type2, id2, size2))

#define LFS_BLOCK_NULL ((lfs_block_t)-1)
#define LFS_BLOCK_INLINE ((lfs_block_t)-2)

typedef uint32_t lfs_tag_t;
typedef int32_t lfs_stag_t;

struct lfs_mattr {
    lfs_tag_t tag;
    const void *buffer;
};

struct lfs_diskoff {
    lfs_block_t block;
    lfs_off_t off;
};

class LittleFS : public FileSystem {
public:
    LittleFS(BlockDevice &block_device) : m_block_device(block_device) {}

    RetType init() {
        RESUME();

        lfs_cfg.context = &m_block_device;
        lfs_cfg.read_size = m_block_device.getBlockSize();
        lfs_cfg.prog_size = m_block_device.getBlockSize();
        lfs_cfg.block_size = m_block_device.getBlockSize();
        lfs_cfg.block_count = m_block_device.getNumBlocks();
        lfs_cfg.block_cycles = 500;
        lfs_cfg.cache_size = 512;
        lfs_cfg.lookahead_size = 512;
        lfs_cfg.name_max = 255;
        lfs_cfg.file_max = 0;
        lfs_cfg.attr_max = 0;

        int err = lfs_mount(&lfs, &lfs_cfg);
        if (err) {
            err = lfs_format(&lfs, &lfs_cfg);
            if (err) {
                RESET();
                return RET_ERROR;
            }

            err = lfs_mount(&lfs, &lfs_cfg);
            if (err) {
                RESET();
                return RET_ERROR;
            }
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType open(const char *filename, int *fd, bool *new_file = NULL) {
        RESUME();


        RESET();
        return RET_SUCCESS;
    }

    RetType write(int fd, uint8_t *buff, size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    RetType read(int fd, uint8_t *buff, size_t len) {
        RESUME();


        RESET();
        return RET_SUCCESS;
    }

    RetType flush(int fd) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    virtual RetType format() {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

private:
    lfs_t lfs;
    lfs_config lfs_cfg;
    BlockDevice &m_block_device;

#ifndef LFS_READONLY

// Format a block device with the littlefs
//
// Requires a littlefs object and config struct. This clobbers the littlefs
// object, and does not leave the filesystem mounted. The config struct must
// be zeroed for defaults and backwards compatibility.
//
// Returns a negative error code on failure.
    int lfs_format() {
        RESUME();

        RetType ret;
        {
            ret = CALL(init());
            if (ret != RET_SUCCESS) return ret;

            // create free lookahead
            memset(lfs.free.buffer, 0, lfs.cfg->lookahead_size);
            lfs.free.off = 0;
            lfs.free.size = lfs_min(8 * lfs.cfg->lookahead_size,
                                    lfs.cfg->block_count);
            lfs.free.i = 0;
            lfs_alloc_ack();

            // create root dir
            static lfs_mdir_t root;
            ret = CALL(lfs_dir_alloc(&root));
            if (ret != RET_SUCCESS) return ret;

            // write one superblock
            static lfs_superblock_t superblock = {
                    .version     = LFS_DISK_VERSION,
                    .block_size  = lfs.cfg->block_size,
                    .block_count = lfs.cfg->block_count,
                    .name_max    = lfs.name_max,
                    .file_max    = lfs.file_max,
                    .attr_max    = lfs.attr_max,
            };

            lfs_superblock_tole32(&superblock);
            err = lfs_dir_commit(lfs, &root, LFS_MKATTRS(
                    { LFS_MKTAG(LFS_TYPE_CREATE, 0, 0), NULL },
                    { LFS_MKTAG(LFS_TYPE_SUPERBLOCK, 0, 8), "littlefs" },
                    {
                        LFS_MKTAG(LFS_TYPE_INLINESTRUCT, 0, sizeof(superblock)),
                        &superblock
                    }));
            if (ret != RET_SUCCESS) goto cleanup;


            // force compaction to prevent accidentally mounting any
            // older version of littlefs that may live on disk
            root.erased = false;
            err = lfs_dir_commit(lfs, &root, NULL, 0);
            if (ret != RET_SUCCESS) goto cleanup;

            // sanity check that fetch works
            err = lfs_dir_fetch(lfs, &root, (const lfs_block_t[2]) {0, 1});
            if (ret != RET_SUCCESS) goto cleanup;

        }

        cleanup:
        lfs_deinit(lfs);
        RESET();
        return ret;
    }

#endif

// Mounts a littlefs
//
// Requires a littlefs object and config struct. Multiple filesystems
// may be mounted simultaneously with multiple littlefs objects. Both
// lfs and config must be allocated while mounted. The config struct must
// be zeroed for defaults and backwards compatibility.
//
// Returns a negative error code on failure.
    int lfs_mount(lfs_t *lfs, const struct lfs_config *config);

// Unmounts a littlefs
//
// Does nothing besides releasing any allocated resources.
// Returns a negative error code on failure.
    int lfs_unmount(lfs_t *lfs);

/// General operations ///

#ifndef LFS_READONLY

// Removes a file or directory
//
// If removing a directory, the directory must be empty.
// Returns a negative error code on failure.
    int lfs_remove(lfs_t *lfs, const char *path);

#endif

#ifndef LFS_READONLY

// Rename or move a file or directory
//
// If the destination exists, it must match the source in type.
// If the destination is a directory, the directory must be empty.
//
// Returns a negative error code on failure.
    int lfs_rename(lfs_t *lfs, const char *oldpath, const char *newpath);

#endif

// Find info about a file or directory
//
// Fills out the info structure, based on the specified file or directory.
// Returns a negative error code on failure.
    int lfs_stat(lfs_t *lfs, const char *path, struct lfs_info *info);

// Get a custom attribute
//
// Custom attributes are uniquely identified by an 8-bit type and limited
// to LFS_ATTR_MAX bytes. When read, if the stored attribute is smaller than
// the buffer, it will be padded with zeros. If the stored attribute is larger,
// then it will be silently truncated. If no attribute is found, the error
// LFS_ERR_NOATTR is returned and the buffer is filled with zeros.
//
// Returns the size of the attribute, or a negative error code on failure.
// Note, the returned size is the size of the attribute on disk, irrespective
// of the size of the buffer. This can be used to dynamically allocate a buffer
// or check for existence.
    lfs_ssize_t lfs_getattr(lfs_t *lfs, const char *path,
                            uint8_t type, void *buffer, lfs_size_t size);

#ifndef LFS_READONLY

// Set custom attributes
//
// Custom attributes are uniquely identified by an 8-bit type and limited
// to LFS_ATTR_MAX bytes. If an attribute is not found, it will be
// implicitly created.
//
// Returns a negative error code on failure.
    int lfs_setattr(lfs_t *lfs, const char *path,
                    uint8_t type, const void *buffer, lfs_size_t size);

#endif

#ifndef LFS_READONLY

// Removes a custom attribute
//
// If an attribute is not found, nothing happens.
//
// Returns a negative error code on failure.
    int lfs_removeattr(lfs_t *lfs, const char *path, uint8_t type);

#endif


/// File operations ///

#ifndef LFS_NO_MALLOC
    // Open a file
    //
    // The mode that the file is opened in is determined by the flags, which
    // are values from the enum lfs_open_flags that are bitwise-ored together.
    //
    // Returns a negative error code on failure.
    int lfs_file_open(lfs_t *lfs, lfs_file_t *file,
            const char *path, int flags);

    // if LFS_NO_MALLOC is defined, lfs_file_open() will fail with LFS_ERR_NOMEM
    // thus use lfs_file_opencfg() with config.buffer set.
#endif

// Open a file with extra configuration
//
// The mode that the file is opened in is determined by the flags, which
// are values from the enum lfs_open_flags that are bitwise-ored together.
//
// The config struct provides additional config options per file as described
// above. The config struct must remain allocated while the file is open, and
// the config struct must be zeroed for defaults and backwards compatibility.
//
// Returns a negative error code on failure.
    int lfs_file_opencfg(lfs_t *lfs, lfs_file_t *file,
                         const char *path, int flags,
                         const struct lfs_file_config *config);

// Close a file
//
// Any pending writes are written out to storage as though
// sync had been called and releases any allocated resources.
//
// Returns a negative error code on failure.
    int lfs_file_close(lfs_t *lfs, lfs_file_t *file);

// Synchronize a file on storage
//
// Any pending writes are written out to storage.
// Returns a negative error code on failure.
    int lfs_file_sync(lfs_t *lfs, lfs_file_t *file);

// Read data from file
//
// Takes a buffer and size indicating where to store the read data.
// Returns the number of bytes read, or a negative error code on failure.
    lfs_ssize_t lfs_file_read(lfs_t *lfs, lfs_file_t *file,
                              void *buffer, lfs_size_t size);

#ifndef LFS_READONLY

// Write data to file
//
// Takes a buffer and size indicating the data to write. The file will not
// actually be updated on the storage until either sync or close is called.
//
// Returns the number of bytes written, or a negative error code on failure.
    lfs_ssize_t lfs_file_write(lfs_t *lfs, lfs_file_t *file,
                               const void *buffer, lfs_size_t size);

#endif

// Change the position of the file
//
// The change in position is determined by the offset and whence flag.
// Returns the new position of the file, or a negative error code on failure.
    lfs_soff_t lfs_file_seek(lfs_t *lfs, lfs_file_t *file,
                             lfs_soff_t off, int whence);

#ifndef LFS_READONLY

// Truncates the size of the file to the specified size
//
// Returns a negative error code on failure.
    int lfs_file_truncate(lfs_t *lfs, lfs_file_t *file, lfs_off_t size);

#endif

// Return the position of the file
//
// Equivalent to lfs_file_seek(lfs, file, 0, LFS_SEEK_CUR)
// Returns the position of the file, or a negative error code on failure.
    lfs_soff_t lfs_file_tell(lfs_t *lfs, lfs_file_t *file);

// Change the position of the file to the beginning of the file
//
// Equivalent to lfs_file_seek(lfs, file, 0, LFS_SEEK_SET)
// Returns a negative error code on failure.
    int lfs_file_rewind(lfs_t *lfs, lfs_file_t *file);

// Return the size of the file
//
// Similar to lfs_file_seek(lfs, file, 0, LFS_SEEK_END)
// Returns the size of the file, or a negative error code on failure.
    lfs_soff_t lfs_file_size(lfs_t *lfs, lfs_file_t *file);


/// Directory operations ///

#ifndef LFS_READONLY

// Create a directory
//
// Returns a negative error code on failure.
    int lfs_mkdir(lfs_t *lfs, const char *path);

#endif

// Open a directory
//
// Once open a directory can be used with read to iterate over files.
// Returns a negative error code on failure.
    int lfs_dir_open(lfs_t *lfs, lfs_dir_t *dir, const char *path);

// Close a directory
//
// Releases any allocated resources.
// Returns a negative error code on failure.
    int lfs_dir_close(lfs_t *lfs, lfs_dir_t *dir);

// Read an entry in the directory
//
// Fills out the info structure, based on the specified file or directory.
// Returns a positive value on success, 0 at the end of directory,
// or a negative error code on failure.
    int lfs_dir_read(lfs_t *lfs, lfs_dir_t *dir, struct lfs_info *info);

// Change the position of the directory
//
// The new off must be a value previous returned from tell and specifies
// an absolute offset in the directory seek.
//
// Returns a negative error code on failure.
    int lfs_dir_seek(lfs_t *lfs, lfs_dir_t *dir, lfs_off_t off);

// Return the position of the directory
//
// The returned offset is only meant to be consumed by seek and may not make
// sense, but does indicate the current position in the directory iteration.
//
// Returns the position of the directory, or a negative error code on failure.
    lfs_soff_t lfs_dir_tell(lfs_t *lfs, lfs_dir_t *dir);

// Change the position of the directory to the beginning of the directory
//
// Returns a negative error code on failure.
    int lfs_dir_rewind(lfs_t *lfs, lfs_dir_t *dir);


/// Filesystem-level filesystem operations

// Finds the current size of the filesystem
//
// Note: Result is best effort. If files share COW structures, the returned
// size may be larger than the filesystem actually is.
//
// Returns the number of allocated blocks, or a negative error code on failure.
    lfs_ssize_t lfs_fs_size(lfs_t *lfs);

// Traverse through all blocks in use by the filesystem
//
// The provided callback will be called with each block address that is
// currently in use by the filesystem. This can be used to determine which
// blocks are in use or how much of the storage is available.
//
// Returns a negative error code on failure.
    int lfs_fs_traverse(lfs_t *lfs, int (*cb)(void *, lfs_block_t), void *data);

#ifndef LFS_READONLY
#ifdef LFS_MIGRATE
    int lfs_migrate(lfs_t *lfs, const struct lfs_config *cfg);
#endif
#endif

    RetType lfs_bd_read(const lfs_cache_t *pcache, lfs_cache_t *rcache, lfs_size_t hint,
                        lfs_block_t block, lfs_off_t off,
                        void *buffer, lfs_size_t size) {
        RESUME();
        static uint8_t *data = buffer;
        if (block >= lfs.cfg->block_count ||
            off + size > lfs.cfg->block_size) {
            return RET_ERROR; // Corrupt
        }

        while (size > 0) {
            static lfs_size_t diff = size;

            if (pcache && block == pcache->block &&
                off < pcache->off + pcache->size) {
                if (off >= pcache->off) {
                    // is already in pcache?
                    diff = lfs_min(diff, pcache->size - (off - pcache->off));
                    memcpy(data, &pcache->buffer[off - pcache->off], diff);

                    data += diff;
                    off += diff;
                    size -= diff;
                    continue;
                }

                // pcache takes priority
                diff = lfs_min(diff, pcache->off - off);
            }

            if (block == rcache->block &&
                off < rcache->off + rcache->size) {
                if (off >= rcache->off) {
                    // is already in rcache?
                    diff = lfs_min(diff, rcache->size - (off - rcache->off));
                    memcpy(data, &rcache->buffer[off - rcache->off], diff);

                    data += diff;
                    off += diff;
                    size -= diff;
                    continue;
                }

                // rcache takes priority
                diff = lfs_min(diff, rcache->off - off);
            }

            if (size >= hint && off % lfs.cfg->read_size == 0 &&
                size >= lfs.cfg->read_size) {
                // bypass cache?
                diff = lfs_aligndown(diff, lfs.cfg->read_size);
                RetType ret = CALL(lfs.cfg->read(lfs.cfg, block, off, data, diff));
                if (ret != RET_SUCCESS) return ret;

                data += diff;
                off += diff;
                size -= diff;
                continue;
            }

            // load to cache, first condition can no longer fail
            LFS_ASSERT(block < lfs.cfg->block_count);
            rcache->block = block;
            rcache->off = lfs_aligndown(off, lfs.cfg->read_size);
            rcache->size = lfs_min(
                    lfs_min(lfs_alignup(off + hint, lfs.cfg->read_size), lfs.cfg->block_size) - rcache->off,
                    lfs.cfg->cache_size);
            RetType ret = CALL(lfs.cfg->read(lfs.cfg, rcache->block,
                                             rcache->off, rcache->buffer, rcache->size));
            if (ret != RET_SUCCESS) return ret;
        }

        RESET();
        return RET_SUCCESS;
    }

    int lfs_fs_rawtraverse(int (*cb)(void *data, lfs_block_t block), void *data, bool includeorphans) {
        // iterate over metadata pairs
        lfs_mdir_t dir = {.tail = {0, 1}};

#ifdef LFS_MIGRATE
        // also consider v1 blocks during migration
        if (lfs.lfs1) {
            int err = lfs1_traverse(lfs, cb, data);
            if (err) {
                return err;
            }
    
            dir.tail[0] = lfs.root[0];
            dir.tail[1] = lfs.root[1];
        }
#endif

        lfs_block_t cycle = 0;
        while (!lfs_pair_isnull(dir.tail)) {
            if (cycle >= lfs.cfg->block_count / 2) {
                // loop detected
                return LFS_ERR_CORRUPT;
            }
            cycle += 1;

            for (int i = 0; i < 2; i++) {
                int err = cb(data, dir.tail[i]);
                if (err) {
                    return err;
                }
            }

            // iterate through ids in directory
            int err = lfs_dir_fetch(lfs, &dir, dir.tail);
            if (err) {
                return err;
            }

            for (uint16_t id = 0; id < dir.count; id++) {
                struct lfs_ctz ctz;
                lfs_stag_t tag = lfs_dir_get(lfs, &dir, LFS_MKTAG(0x700, 0x3ff, 0),
                                             LFS_MKTAG(LFS_TYPE_STRUCT, id, sizeof(ctz)), &ctz);
                if (tag < 0) {
                    if (tag == LFS_ERR_NOENT) {
                        continue;
                    }
                    return tag;
                }
                lfs_ctz_fromle32(&ctz);

                if (lfs_tag_type3(tag) == LFS_TYPE_CTZSTRUCT) {
                    err = lfs_ctz_traverse(lfs, NULL, &lfs.rcache,
                                           ctz.head, ctz.size, cb, data);
                    if (err) {
                        return err;
                    }
                } else if (includeorphans &&
                           lfs_tag_type3(tag) == LFS_TYPE_DIRSTRUCT) {
                    for (int i = 0; i < 2; i++) {
                        err = cb(data, (&ctz.head)[i]);
                        if (err) {
                            return err;
                        }
                    }
                }
            }
        }

#ifndef LFS_READONLY
        // iterate over any open files
        for (lfs_file_t *f = (lfs_file_t *) lfs.mlist; f; f = f->next) {
            if (f->type != LFS_TYPE_REG) {
                continue;
            }

            if ((f->flags & LFS_F_DIRTY) && !(f->flags & LFS_F_INLINE)) {
                int err = lfs_ctz_traverse(lfs, &f->cache, &lfs.rcache,
                                           f->ctz.head, f->ctz.size, cb, data);
                if (err) {
                    return err;
                }
            }

            if ((f->flags & LFS_F_WRITING) && !(f->flags & LFS_F_INLINE)) {
                int err = lfs_ctz_traverse(lfs, &f->cache, &lfs.rcache,
                                           f->block, f->pos, cb, data);
                if (err) {
                    return err;
                }
            }
        }
#endif

        return 0;
    }


#ifndef LFS_READONLY

    RetType lfs_dir_alloc(lfs_mdir_t *dir) {
        RESUME();

        // allocate pair of dir blocks (backwards, so we write block 1 first)
        for (int i = 0; i < 2; i++) {
            RetType ret = lfs_alloc(&dir->pair[(i + 1) % 2]);
            if (ret != RET_SUCCESS) return ret;
        }

        // zero for reproducibility in case initial block is unreadable
        dir->rev = 0;

        // rather than clobbering one of the blocks we just pretend
        // the revision may be valid
        RetType ret = CALL(lfs_bd_read(NULL, &lfs.rcache, sizeof(dir->rev),
                                       dir->pair[0], 0, &dir->rev, sizeof(dir->rev)));
        dir->rev = lfs_fromle32(dir->rev);
        if (ret != RET_SUCCESS) return ret; // TODO: Specific error check


        // to make sure we don't immediately evict, align the new revision count
        // to our block_cycles modulus, see lfs_dir_compact for why our modulus
        // is tweaked this way
        if (lfs.cfg->block_cycles > 0) {
            dir->rev = lfs_alignup(dir->rev, ((lfs.cfg->block_cycles + 1) | 1));
        }

        // set defaults
        dir->off = sizeof(dir->rev);
        dir->etag = 0xffffffff;
        dir->count = 0;
        dir->tail[0] = LFS_BLOCK_NULL;
        dir->tail[1] = LFS_BLOCK_NULL;
        dir->erased = false;
        dir->split = false;

        // don't write out yet, let caller take care of that
        RESET();
        return RET_SUCCESS;
    }

    static int lfs_alloc_lookahead(void *p, lfs_block_t block) {
        lfs_t *lfs = (lfs_t *) p;
        lfs_block_t off = ((block - lfs.free.off)
                           + lfs.cfg->block_count) % lfs.cfg->block_count;

        if (off < lfs.free.size) {
            lfs.free.buffer[off / 32] |= 1U << (off % 32);
        }

        return 0;
    }


    int lfs_alloc(lfs_block_t *block) {
        while (true) {
            while (lfs.free.i != lfs.free.size) {
                lfs_block_t off = lfs.free.i;
                lfs.free.i += 1;
                lfs.free.ack -= 1;

                if (!(lfs.free.buffer[off / 32] & (1U << (off % 32)))) {
                    // found a free block
                    *block = (lfs.free.off + off) % lfs.cfg->block_count;

                    // eagerly find next off so an alloc ack can
                    // discredit old lookahead blocks
                    while (lfs.free.i != lfs.free.size &&
                           (lfs.free.buffer[lfs.free.i / 32]
                            & (1U << (lfs.free.i % 32)))) {
                        lfs.free.i += 1;
                        lfs.free.ack -= 1;
                    }

                    return 0;
                }
            }

            // check if we have looked at all blocks since last ack
            if (lfs.free.ack == 0) {
                LFS_ERROR("No more free space %" PRIu32,
                          lfs.free.i + lfs.free.off);
                return LFS_ERR_NOSPC;
            }

            lfs.free.off = (lfs.free.off + lfs.free.size)
                           % lfs.cfg->block_count;
            lfs.free.size = lfs_min(8 * lfs.cfg->lookahead_size, lfs.free.ack);
            lfs.free.i = 0;

            // find mask of free blocks from tree
            memset(lfs.free.buffer, 0, lfs.cfg->lookahead_size);
            int err = lfs_fs_rawtraverse(lfs, lfs_alloc_lookahead, lfs, true);
            if (err) {
                lfs_alloc_drop();
                return err;
            }
        }
    }

#endif

    void lfs_alloc_ack() {
        lfs.free.ack = lfs.cfg->block_count;
    }

    void lfs_alloc_drop() {
        lfs.free.size = 0;
        lfs.free.i = 0;
        lfs_alloc_ack();
    }

};

#endif //LAUNCH_CORE_LITTLEFS_H
